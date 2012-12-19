#!/bin/bash
cd /store/open/gstreamer/stage

./bin/gst-launch-1.0 -v \
    --gst-debug-no-color \
    --gst-debug="GstTCPMixSrc:5" \
    \
    tcpserversrc name=source port=3000 \
    xvimagesink name=sink \
    gdpdepay name=conv \
    funnel name=fun \
    source. ! fun. \
    fun. ! conv. \
    conv. ! sink.
