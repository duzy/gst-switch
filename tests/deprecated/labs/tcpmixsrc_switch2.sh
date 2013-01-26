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
    switch.src_0 ! fdsink fd=2 \
    switch.src_1 ! fdsink fd=2 \
    switch.src_2 ! fdsink fd=2 \
    switch.src_3 ! fdsink fd=2 \
