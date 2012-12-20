#!/bin/bash
./tests/test-mix-source -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="tcpclientsink:5" \
    --gst-debug="*:3"
