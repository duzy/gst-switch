#!/bin/bash
. ./tests/test.sh

pattern=$1
width=$2
height=$3

if [[ "x$pattern" == "x" ]]; then
    pattern=0
fi

if [[ "x$width" == "x" ]]; then
    width=640
fi

if [[ "x$height" == "x" ]]; then
    height=480
fi

launch -v \
    --gst-debug-no-color \
    videotestsrc pattern=$pattern \
    ! video/x-raw,width=$width,height=$height \
    ! gdppay ! tcpclientsink port=3000
