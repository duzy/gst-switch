#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="switch:5" \
    \
    tcpmixsrc name=source port=3000 autosink=convert \
    convbin name=convert converter=identity autosink=switch \
    switch name=switch autosink=fun \
    fdsink name=sink fd=2 \
    funnel name=fun \
    source. ! switch. \
    fun. ! sink.
