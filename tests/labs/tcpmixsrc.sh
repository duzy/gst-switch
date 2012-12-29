#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    \
    tcpmixsrc name=source port=3000 \
    fdsink name=sink fd=2 \
    funnel name=fun \
    source. ! fun. \
    fun. ! sink.
