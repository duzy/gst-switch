#!/bin/bash
./tests/test-mix-source -v \
    --gst-debug-no-color \
    --gst-debug="GstTCPMixSrc:5,*"
