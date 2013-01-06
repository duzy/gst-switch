#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    audiotestsrc \
    ! audio/x-raw \
    ! gdppay ! tcpclientsink port=4000
