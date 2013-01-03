#!/bin/bash

#gdb --args \
./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --gst-debug="switch:5" \
    --gst-debug="convbin:0" \
    --gst-debug="tcpmixsrc:0" \
    --gst-debug="multiqueue:0" \
    --input-port="3000" \
    --control-port="4000" \
