#! /bin/bash

sudo pip install -r requirements.txt --use-mirrors
echo 'yes' | sudo add-apt-repository ppa:gstreamer-developers/ppa
sudo apt-get update
sudo apt-get install libglib2.0-dev gir1.2-glib-2.0 libgirepository1.0-dev libglib2.0-0 
sudo apt-get install python-gi python3-gi gstreamer1.0-tools gir1.2-gstreamer-1.0 gir1.2-gst-plugins-base-1.0 gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly gstreamer1.0-plugins-bad gstreamer1.0-libav
sudo apt-get install python-scipy ffmpeg
sudo apt-get build-dep gstreamer1.0
sudo apt-get install autoconf automake autopoint libbz2-dev libdv4-dev libfaac-dev libfaad-dev libgtk-3-dev libmjpegtools-dev libtag1-dev libasound2-dev libtool libvpx-dev libxv-dev libx11-dev libogg-dev libvorbis-dev libopencv-dev libcv-dev libhighgui-dev libv4l-dev pkg-config zlib1g-dev gtk-doc-tools yasm bison flex
git clone http://git.chromium.org/webm/libvpx.git
cd libvpx
git checkout v1.2.0
./configure --prefix=/usr/local --enable-shared --enable-vp8
make
sudo make install
cd ..
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
git clone git://anongit.freedesktop.org/gstreamer/gstreamer
cd gstreamer
./autogen.sh --prefix=/usr/local
make
sudo make install
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-base
cd gst-plugins-base
./autogen.sh --prefix=/usr/local
make
sudo make install
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-good
cd gst-plugins-good
./autogen.sh --prefix=/usr/local --enable-experimental
make
sudo make install
cd ..
git clone git://anongit.freedesktop.org/gstreamer/gst-plugins-ugly
cd gst-plugins-ugly
./autogen.sh --prefix=/usr/local
make
sudo make install
cd ..
git clone https://github.com/hyades/gst-plugins-bad.git
cd gst-plugins-bad
./autogen.sh --prefix=/usr/local
make
sudo make install
cd ..

./autogen.sh --prefix=/usr/local
make
sudo make install

if [ $TYPE == 'c' ]; then
	sudo pip install cpp-coveralls
else
	sudo pip install python-coveralls
fi
