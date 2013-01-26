#!/bin/bash
. ./tests/test.sh

launch \
    --gst-debug-no-color \
    videomixer name=mix \
    mix. ! xvimagesink \
    \
    videotestsrc pattern="snow" \
    ! video/x-raw, width=200, height=150 \
    ! videobox border-alpha=0.8 top=-2 bottom=-2 left=-2 right=-2 \
    ! videobox border-alpha=0 alpha=0.6 top=-20 left=-20 \
    ! mix. \
    \
    videotestsrc \
    ! video/x-raw, width=640, height=360 \
    ! mix. \
    

