#!/bin/bash -ex

sudo apt-get install \
    build-essential \
    libglib2.0-dev \
    libgtk-3-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev

# for run-demo.sh 
sudo apt-get install \
    gstreamer1.0-plugins-bad


./autogen.sh
make

