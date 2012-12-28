#!/bin/bash
#cd /store/open/gstreamer/stage

#gdb --args \
./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="switch:5" \
    --gst-debug="convbin:0" \
    --gst-debug="tcpmixsrc:0" \
    --gst-debug="multiqueue:0" \
    --port="3000" \
