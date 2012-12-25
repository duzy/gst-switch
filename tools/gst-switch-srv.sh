#!/bin/bash
#cd /store/open/gstreamer/stage

#gdb --args \
./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="switch:4" \
    --gst-debug="convbin:4" \
    --gst-debug="tcpmixsrc:4" \
    --port="3000" \
