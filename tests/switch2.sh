#!/bin/bash
. ./tests/test.sh

echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4

launch -v \
    --gst-debug-no-color \
    --gst-debug="switch:4" \
    \
    filesrc name=source1 location=/tmp/src1 \
    filesrc name=source2 location=/tmp/src2 \
    filesrc name=source3 location=/tmp/src3 \
    filesrc name=source4 location=/tmp/src4 \
    switch name=switch \
    fdsink name=sink1 fd=2 \
    fdsink name=sink2 fd=2 \
    source1. ! switch. \
    source2. ! switch. \
    source3. ! switch. \
    source4. ! switch. \
    switch. ! sink1. \
    switch. ! sink2. \
