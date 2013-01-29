#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
function goto_gstreamer_stage_directory() {
    local D1=/store/open/gstreamer/stage
    local D2=$HOME/gst/stage
    [ -d $D1 ] && cd $D1 || cd $D2
}

function launch() {
    goto_gstreamer_stage_directory
    ./bin/gst-launch-1.0 $@
    cd - > /dev/null
}

function check_file() {
    echo "=================================================="
    echo "GOT:"
    cat $1
    echo "--------------------------------------------------"
    echo "EXPECT:"
    echo -n "$2"
    echo "=================================================="
}
