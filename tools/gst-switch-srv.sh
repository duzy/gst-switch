#!/bin/bash
#cd /store/open/gstreamer/stage

./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="GstTCPMixSrc:5" \
    --port="3000" \
