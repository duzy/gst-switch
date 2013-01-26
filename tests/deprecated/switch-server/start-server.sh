#!/bin/bash
. ./tests/test.sh

./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="switch:5" \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="videomixer:3" \
    --gst-debug="gdpdepay:4" \
    --gst-debug="multiqueue:4" \
    --gst-debug="input-selector:3" \
    --port=3000
