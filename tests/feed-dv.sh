#!/bin/bash
#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./tests/test.sh

dvfile="$1"
width="$2"
height="$3"

if [[ "x$pattern" == "x" ]]; then
    pattern=0
fi

if [[ "x$width" == "x" ]]; then
    #width=640
    #width=1280
    width=300
fi

if [[ "x$height" == "x" ]]; then
    #height=480
    #height=768
    height=200
fi

launch -v \
    --gst-debug-no-color \
    filesrc location="$dvfile" \
    ! dvdemux name=dmx ! dvdec ! videoconvert \
    ! videoscale ! video/x-raw,format=I420,width=$width,height=$height \
    ! gdppay ! tcpclientsink port=3000
