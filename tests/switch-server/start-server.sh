#!/bin/bash
. ./tests/test.sh

./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="switch:5" \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="funnel:5" \
    --gst-debug="tee:5" \
    --gst-debug="videomixer:5" \
    --gst-debug="gdpdepay:5" \
    --gst-debug="multiqueue:4" \
    --port=3000
