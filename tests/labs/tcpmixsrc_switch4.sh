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
    identity name=fun0 ! fdsink fd=2 \
    identity name=fun1 ! fdsink fd=2 \
    switch.src_0 ! fun0. \
    switch.src_1 ! fun1. \
