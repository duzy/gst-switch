#!/bin/bash
. ./tests/test.sh

echo "1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" > /tmp/src1
echo "2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2" > /tmp/src2
echo "3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3" > /tmp/src3
echo "4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4" > /tmp/src4

function send_file() {
    launch -v \
	--gst-debug-no-color \
	--gst-debug="tcpmixsrc:5" \
	\
	filesrc name=source location=$1 \
	tcpclientsink name=sink port=3000 \
	source. ! sink. \
	1>/dev/null 2>/dev/null
}

for I in /tmp/src[1-4] ; do
    send_file $I
done
