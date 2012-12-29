#!/bin/bash
. ./tests/test.sh

echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src0
echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4


if false; then
    launch -v \
	--gst-debug-no-color \
	filesrc location=/tmp/src1 ! fun. \
	filesrc location=/tmp/src2 ! fun. \
	filesrc location=/tmp/src3 ! fun. \
	filesrc location=/tmp/src4 ! fun. \
	funnel name=fun ! tcpclientsink port=3000
else
    launch -v \
	--gst-debug-no-color \
	filesrc location=/tmp/src0 ! tcpclientsink port=3000
fi
