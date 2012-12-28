#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    tcpclientsrc port=3001 ! gdpdepay ! xvimagesink
