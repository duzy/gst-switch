#!/bin/bash
cd /store/open/gstreamer/stage

echo "1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4

./bin/gst-launch-1.0 -v \
    --gst-debug-no-color \
    --gst-debug="switch:5" \
    --gst-debug="*.5" \
    \
    filesrc name=source1 location=/tmp/src1 \
    switch name=s "case=+[1,2]" \
    fdsink name=sink fd=2 \
    source1. ! s. \
    s. ! funnel ! sink.
