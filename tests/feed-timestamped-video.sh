#!/bin/bash
#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./tests/test.sh

pattern=$1
width=$2
height=$3

if [[ "x$pattern" == "x" ]]; then
    pattern=0
fi

if [[ "x$width" == "x" ]]; then
    #width=640
    #width=1280
    #width=356
    width=300
fi

if [[ "x$height" == "x" ]]; then
    #height=480
    #height=720
    #height=240
    height=200
fi

launch \
    --gst-debug-no-color \
    videotestsrc pattern=$pattern \
    ! video/x-raw,width=$width,height=$height \
    ! timeoverlay font-desc='"Verdana bold 62"' ! tee name=v \
    v. ! queue ! textoverlay font-desc='"Sans 90"' text=111 \
       ! gdppay ! tcpclientsink name=tcp_sink1 port=3000 \
    v. ! queue ! textoverlay font-desc='"Sans 90"' text=222 \
       ! gdppay ! tcpclientsink name=tcp_sink2 port=3000 \
    v. ! queue ! textoverlay font-desc='"Sans 90"' text=333 \
       ! gdppay ! tcpclientsink name=tcp_sink3 port=3000 \
    v. ! queue ! textoverlay font-desc='"Sans 90"' text=444 \
       ! gdppay ! tcpclientsink name=tcp_sink4 port=3000
