#!/bin/bash
cd /store/open/gstreamer/stage

./bin/gst-launch-1.0 -v \
    --gst-debug-no-color \
    --gst-debug="GstTCPMixSrc:5" \
    \
    videotestsrc ! gdppay ! tcpclientsink port=3000
