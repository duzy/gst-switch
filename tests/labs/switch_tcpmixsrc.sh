#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="switch:5" \
    \
    tcpmixsrc name=source port=3000 \
    switch name=switch \
    fdsink name=sink fd=2 \
    funnel name=fun \
    source. ! switch. \
    switch. ! fun. \
    fun. ! sink.
