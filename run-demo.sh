#!/bin/bash -ex

cd tools

./gst-switch-srv &
sleep 5
./gst-switch-ui &
sleep 5
gst-launch-1.0 videotestsrc pattern=1 is-live=1 \
        ! timeoverlay \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
&

sleep 20

gst-launch-1.0 videotestsrc pattern=18 is-live=1 \
        ! timeoverlay \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
&


