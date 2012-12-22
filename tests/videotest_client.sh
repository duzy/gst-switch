#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    \
    videotestsrc ! gdppay ! tcpclientsink port=3000
