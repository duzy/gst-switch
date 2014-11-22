#!/bin/bash -ex

# demos issue:  lag between thumb and canvas #30 

cd ../tools

./gst-switch-srv & srvpid=$! 
sleep 5
./gst-switch-ui & uipid=$!
sleep 5
gst-launch-1.0 videotestsrc pattern=1 is-live=1 \
        ! timeoverlay \
        ! clockoverlay time-format="%S" font-desc="Sans 240" \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
& src1pid=$!

sleep 20

gst-launch-1.0 videotestsrc pattern=18 is-live=1 \
        ! timeoverlay \
        ! clockoverlay time-format="%S" font-desc="Sans 240" \
        ! video/x-raw, width=300, height=200 \
        ! gdppay \
        ! tcpclientsink port=3000 \
& src2pid=$!

pgrep gst
echo $srvpid $uipid $src1pid $src2pid

read -p "press enter to kill all the processes:"

kill $srvpid $uipid $src1pid $src2pid
pgrep gst

