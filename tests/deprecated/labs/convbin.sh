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
    convbin name=conv converter=identity autosink=sum \
    filesrc location=/tmp/src1 ! conv. \
    filesrc location=/tmp/src2 ! conv. \
    filesrc location=/tmp/src3 ! conv. \
    filesrc location=/tmp/src4 ! conv. \
    funnel name="sum" ! fdsink name=sink fd=2 \
