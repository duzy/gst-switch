#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    --gst-debug="tcpmixsrc:5" \
    --gst-debug="switch:5" \
    \
    tcpmixsrc name=source port=3000 autosink=switch mode=loop \
    switch name=switch \
    source. ! switch. \
    xvimagesink name=sink0 \
    xvimagesink name=sink1 \
    xvimagesink name=sink2 \
    switch.src_0 ! sink0. \
    switch.src_1 ! sink1. \
    switch.src_2 ! sink2. \
