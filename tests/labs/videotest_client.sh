#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    videotestsrc ! gdppay ! tcpclientsink port=3000
