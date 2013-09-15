#! /bin/bash -ex

sudo apt-get -y install python-software-properties python-pip
sudo pip install -r requirements.txt --use-mirrors
echo 'yes' | sudo add-apt-repository ppa:gstreamer-developers/ppa
echo 'yes' | sudo add-apt-repository ppa:theora/ppa
sudo apt-get update
sudo apt-get -y install libglib2.0-dev gir1.2-glib-2.0 libgirepository1.0-dev libglib2.0-0 
sudo apt-get -y install python-gi python3-gi gstreamer1.0-tools gir1.2-gstreamer-1.0 gir1.2-gst-plugins-base-1.0 gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly gstreamer1.0-plugins-bad gstreamer1.0-libav
sudo apt-get -y install python-scipy ffmpeg
sudo apt-get -y build-dep gstreamer1.0
sudo apt-get -y install autoconf automake autopoint libbz2-dev libdv4-dev libfaac-dev libfaad-dev libgtk-3-dev libmjpegtools-dev libtag1-dev libasound2-dev libtool libvpx-dev libxv-dev libx11-dev libogg-dev libvorbis-dev libopencv-dev libcv-dev libhighgui-dev libv4l-dev pkg-config zlib1g-dev gtk-doc-tools yasm bison flex
sudo apt-get -y install screen openssh-server nfs-common bpython git-core subversion git-svn build-essential xclip curl python-setuptools mercurial libqt4-dev installation-guide-i386 gdb libc-dbg kerneloops python-wxversion libboost-dev libboost-thread-dev libgtkmm-2.4-dev libxv-dev cmake libasound2-dev autotools-dev libltdl7-dev m4 kexec-tools tftp iotop iftop ffmpeg bpython ipython ack-grep libavcodec-dev kino imagemagick mplayer vlc dvsink dvsource dvswitch ffmpeg2theora git automake autoconf libtool intltool g++ bison yasm swig libmp3lame-dev libsamplerate-dev libxml2-dev libjack-dev libsox-dev libgtk2.0-dev libexif-dev libvdpau-dev python-dev sox gconf-editor liboil-dev sshfs oggfwd sshfs libdv-bin python-demjson libgavl-dev ladspa-sdk libsdl-dev libqt4-dev libtheora-dev libvorbis-dev python-dev libvpx-dev liboil0.3-dev libfaac0 libfaac-dev python-gtk2 python-gst0.10 gstreamer0.10-plugins-good gstreamer0.10-plugins-bad gocr imagemagick python-imaging python-reportlab python-pip mercurial subversion inkscape vim mencoder ffmpeg python-virtualenv screen sox dconf-tools gscanbus 

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
export LD_LIBRARY_PATH=/usr/local/lib/
git clone http://git.chromium.org/webm/libvpx.git
cd libvpx
git checkout v1.2.0
./configure --enable-shared --enable-vp8 --prefix=/usr/local
[[ -f Makefile ]] || {
	printf "Configure libvpx failed, no Makefile generated!!!\n"
	exit -1
    }
make
sudo make install || {
       printf "make install of libvpx failed!!!\n"
       exit -1
    }
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gstreamer
cd gstreamer
git checkout 6c11da1
./autogen.sh --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
[[ -f Makefile ]] || {
	printf "Configure gstreamer failed, no Makefile generated!!!\n"
	exit -1
    }
make clean
make || {
   printf "make of $project failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of gstreamer failed!!!\n"
       exit -1
    }
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-base
cd gst-plugins-base
git checkout a8df760
./autogen.sh --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
[[ -f Makefile ]] || {
	printf "Configure gst-plugins-base failed, no Makefile generated!!!\n"
	exit -1
    }
make clean
make || {
   printf "make of gst-plugins-base failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of $project failed!!!\n"
       exit -1
    }
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-good
cd gst-plugins-good
git checkout d14d4c4
./autogen.sh --enable-experimental --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
[[ -f Makefile ]] || {
	printf "Configure gst-plugins-good failed, no Makefile generated!!!\n"
	exit -1
    }
make clean
make || {
   printf "make of gst-plugins-good failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of $project failed!!!\n"
       exit -1
    }
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-ugly
cd gst-plugins-ugly
git checkout 68985ba
./autogen.sh --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
[[ -f Makefile ]] || {
	printf "Configure gst-plugins-ugly failed, no Makefile generated!!!\n"
	exit -1
    }
make clean
make || {
   printf "make of gst-plugins-ugly failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of gst-plugins-ugly failed!!!\n"
       exit -1
    }
cd ..
git clone https://github.com/duzy/gst-plugins-bad.git
cd gst-plugins-bad
git checkout speakertrack_new
git checkout 
./autogen.sh --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
[[ -f Makefile ]] || {
	printf "Configure gst-plugins-bad failed, no Makefile generated!!!\n"
	exit -1
    }
make clean
make || {
   printf "make of gst-plugins-bad failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of gst-plugins-bad failed!!!\n"
       exit -1
    }
cd ..
./autogen.sh --prefix=/usr/local || {
printf "Failed to do autogen!!!\n"
exit -1
}
make clean
make || {
   printf "make of gstswitch failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of gstswitch failed!!!\n"
       exit -1
    }
# sudo cp /usr/local/lib/girepository-1.0/*.* /usr/lib/girepository-1.0/

if [ $TYPE == 'c' ]; then
	sudo pip install cpp-coveralls
else
	sudo pip install python-coveralls
fi
