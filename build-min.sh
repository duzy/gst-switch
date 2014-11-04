!#/bin/bash -ex

sudo apt-get install \
    libglib2.0-dev \
    libgtk-3-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev

./autogen.sh
make

