#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    \
    tcpmixsrc name=source port=3000 \
    xvimagesink name=sink \
    gdpdepay name=conv \
    funnel name=fun \
    source. ! fun. \
    fun. ! conv. \
    conv. ! sink.
