#! /bin/bash -ex

sudo apt-get update
sudo pip install mock --upgrade
sudo pip install pytest-cov --upgrade
sudo pip install pytest-pep8 --upgrade
sudo pip install pylint --upgrade
sudo apt-get -y install libglib2.0-dev gir1.2-glib-2.0 libgirepository1.0-dev libglib2.0-0 python-gi
sudo apt-get -y install gstreamer0.10-plugins-good
sudo apt-get -y install python-scipy
sudo apt-get -y install ffmpeg || sudo apt-get -y install libav-tools
sudo apt-get -y install libvo-aacenc-dev
sudo apt-get -y install autoconf automake autopoint libbz2-dev libdv4-dev libfaac-dev libfaad-dev libgtk-3-dev libmjpegtools-dev libtag1-dev libasound2-dev libtool libvpx-dev libxv-dev libx11-dev libogg-dev libvorbis-dev libopencv-dev libcv-dev libhighgui-dev libv4l-dev pkg-config zlib1g-dev gtk-doc-tools yasm bison flex

./autogen.sh --prefix=/usr || {
printf "Failed to do autogen!!!\n"
exit -1
}
make clean
make|| {
   printf "make of gstswitch failed!!!\n"
   exit -1
}
sudo make install || {
       printf "make install of gstswitch failed!!!\n"
       exit -1
    }

if [ $TYPE == 'c' ]; then
	sudo pip install cpp-coveralls
else
	sudo pip install python-coveralls
fi
