#!/bin/bash
. ./tests/test.sh

echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4

launch -v \
    --gst-debug-no-color \
    --gst-debug="convbin:5" \
    \
    filesrc name=source1 location=/tmp/src1 \
    filesrc name=source2 location=/tmp/src2 \
    filesrc name=source3 location=/tmp/src3 \
    filesrc name=source4 location=/tmp/src4 \
    convbin name=conv converter=identity \
    fdsink name=sink fd=2 \
    funnel name="fun" \
    source1. ! conv. \
    source2. ! conv. \
    source3. ! conv. \
    source4. ! conv. \
    conv. ! fun. \
    fun. ! sink.
