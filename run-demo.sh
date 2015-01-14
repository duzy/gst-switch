#!/bin/bash -ex

# FIXME: Rewrite using the Python API so this is reliable.

killall gst-switch-srv || true
sleep 2
./tools/gst-switch-srv -f debug -r &
sleep 2
./tools/gst-switch-ui &
sleep 2

gst-launch-1.0 audiotestsrc is-live=true \
        ! audioconvert \
        ! audio/x-raw,rate=48000,channels=2,format=S16LE,layout=interleaved \
        ! gdppay \
        ! tcpclientsink port=4000 \
&

gst-launch-1.0 videotestsrc pattern=1 is-live=1 \
        ! timeoverlay \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
&

gst-launch-1.0 videotestsrc pattern=18 is-live=1 \
        ! timeoverlay \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
&

wait
killall gst-switch-srv || true
killall gst-switch-ui || true
