#!/bin/bash
. ./tests/test.sh

if true; then
    launch -v \
	--gst-debug-no-color \
	tcpclientsrc port=$1 ! avidemux name=demux \
	demux.video_0 ! vp8dec ! videoconvert ! xvimagesink \
	demux.audio_0 ! faad ! audioconvert ! autoaudiosink
else
    printf "Source port: %d\n" $1
    launch -v \
	--gst-debug-no-color \
	--gst-debug="tcpserversink:5" \
	tcpclientsrc port=$1 ! gdpdepay ! filesink location=encode.dat
fi
