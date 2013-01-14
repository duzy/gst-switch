#!/bin/bash

function gst-root()
{
    echo $HOME/gst
}

function gst-stage()
{
    echo $(gst-root)/stage
}

function get-pkg-info()
{
    dpkg -l $1 | grep $1 | awk '{ print $2 " " $3; }'
}

function install-prerequisite()
{
    local pkg=$1
    local pkginfo=$(get-pkg-info $pkg)
    if [[ "x$pkginfo" == "x" ]]; then
	printf "install $pkginfo..\n"
	sudo apt-get install $pkg
    else
	printf "package $pkg is ok\n"
    fi
}

function prepare-prerequisites()
{
    for name in \
	libmjpegtools-dev \
	libvpx-dev \
	libgtk-3-dev \
	;
    do
	install-prerequisite $name
    done
}

function goto-gst-root()
{
    local gstroot=$(gst-root)
    mkdir -p $gstroot
    cd $gstroot
}

function clone-project()
{
    local reporoot=$1
    local project=$2
    local suffix=$3
    local depth=1
    local DEPTH="--depth=$depth"
    local repo="$reporoot/$project$suffix"

    DEPTH=

    if [[ -d $project/.git ]]; then
	cd $project || {
	    exit -1
	}
	echo "Update $project..."
	git pull $DEPTH
	cd - > /dev/null
    else
	rm -rf $project
	#echo "Clone $project..."
	git clone $DEPTH $repo
    fi
}

function clone-duzy-project()
{
    clone-project https://github.com/duzy $1 .git
}

function clone-gst-project()
{
    clone-project git://anongit.freedesktop.org/gstreamer $1
}

function build-project()
{
    local project=$1
    local stage=$(gst-stage)
    local backdir=$PWD
    cd $project
    printf "Building $project...\n" 
    if [[ ! -f Makefile ]]; then
	if [[ -f autoregen.sh ]]; then
	    ./autoregen.sh
	else
	    ./autogen.sh --prefix=$stage || {
		printf "Failed to do autogen!!!\n"
		exit -1
	    }
	fi
    fi
    [[ -f Makefile ]] || {
	printf "Configure $project failed, no Makefile generated!!!\n"
	exit -1
    }
    make && make install
    cd $backdir
}

function build-gst-project()
{
    local project=$1
    build-project $project
    for i in stage/lib/pkgconfig/gstreamer-*; do
	sudo ln -svf $PWD/$i /usr/lib/pkgconfig
    done
}

function prepare-gst-projects()
{
    for name in $@; do
	clone-gst-project $name
	build-gst-project $name
    done
}

function parse-options()
{
    declare -A options
    for arg in $@; do
	case $arg in
	    --force|-f)
		options[force]="yes"
		;;
	esac
    done
    for k in ${!options[@]}; do
	printf "[$k]=\"${options[$k]}\" "
    done
}

function main()
{
    declare -A options="( $(parse-options $@) )"
    local back=$PWD
    local stage=$(gst-stage)
    local force=no

    if  [[ ! -f $stage/bin/gst-launch-1.0 ]] ||
	[[ "x${options[force]}" == "xyes" ]]; then
	prepare-prerequisites
	goto-gst-root
	prepare-gst-projects \
	    gstreamer \
	    gst-plugins-base \
	    gst-plugins-good \
	    gst-plugins-bad \
	    gst-plugins-ugly
    fi

    if  [[ -f $back/../gst-switch/scripts/app_stage.sh ]] &&
	[[ -s $back/../gst-switch/scripts/stage ]]; then
	cd $back && build-project .
    else
	clone-duzy-project gst-switch
	build-project gst-switch
    fi
}
