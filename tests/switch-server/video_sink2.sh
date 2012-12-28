#!/bin/bash
. ./tests/test.sh

launch -v \
    --gst-debug-no-color \
    tcpclientsrc port=3002 ! gdpdepay ! xvimagesink

