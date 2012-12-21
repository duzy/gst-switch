#!/bin/bash
cd /store/open/gstreamer/stage

./bin/gst-launch-1.0 -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="switch:5" \
    \
    tcpmixsrc name=source port=3000 \
    switch name=switch cases="[[1,2]]" \
    fdsink name=sink fd=2 \
    funnel name=fun \
    source. ! switch. \
    switch. ! fun. \
    fun. ! sink.
