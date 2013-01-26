#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    fdsrc fd=1 ! tcpclientsink port=3000
