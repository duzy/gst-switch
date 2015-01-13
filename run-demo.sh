#!/bin/bash -ex

# FIXME: Rewrite using the Python API so this is reliable.

VIDEO_CAPS="video/x-raw, format=(string)I420, pixel-aspect-ratio=(fraction)1/1, width=(int)300, height=(int)200, framerate=(fraction)25/1"
AUDIO_CAPS="audio/x-raw, rate=48000, channels=2, format=S16LE, layout=interleaved"

killall gst-switch-srv || true
sleep 2
./tools/gst-switch-srv -f debug -r &
sleep 2
./tools/gst-switch-ui &
sleep 2

gst-launch-1.0 audiotestsrc is-live=true \
        ! audioconvert \
        ! $AUDIO_CAPS \
        ! gdppay \
        ! tcpclientsink port=4000 \
&

gst-launch-1.0 videotestsrc pattern=1 is-live=1 \
        ! timeoverlay \
        ! $VIDEO_CAPS \
        ! gdppay \
        ! tcpclientsink port=3000 \
&

gst-launch-1.0 videotestsrc pattern=18 is-live=1 \
        ! timeoverlay \
        ! $VIDEO_CAPS \
        ! gdppay \
        ! tcpclientsink port=3000 \
&

wait
killall gst-switch-srv || true
killall gst-switch-ui || true
