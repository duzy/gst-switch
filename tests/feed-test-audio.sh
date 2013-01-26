#!/bin/bash
#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    audiotestsrc \
    ! audio/x-raw \
    ! gdppay ! tcpclientsink port=4000
