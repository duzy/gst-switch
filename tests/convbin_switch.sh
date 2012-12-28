#!/bin/bash
. ./tests/test.sh

echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4

launch -v \
    --gst-debug-no-color \
    --gst-debug="switch:0" \
    --gst-debug="convbin:0" \
    \
    convbin name=conv converter=identity autosink=switch \
    filesrc location=/tmp/src1 ! conv. \
    filesrc location=/tmp/src2 ! conv. \
    filesrc location=/tmp/src3 ! conv. \
    filesrc location=/tmp/src4 ! conv. \
    switch name=switch ! funnel ! fdsink fd=2

#    fdsink name=sink1 fd=2 \
#    fdsink name=sink2 fd=2 \
#    switch. ! sink1. \
#    switch. ! sink2. \
