#!/bin/bash -ex

# test_ui_lag1.sh
# demos issue #30: lag between thumb and canvas 

cd ../tools

./gst-switch-srv & srvpid=$! 
sleep 5
./gst-switch-ui & uipid=$!

# srcpid=()
for i in 1 2 3 4 5 6; do
  sleep 2
  gst-launch-1.0 videotestsrc pattern=18 is-live=1 \
    ! textoverlay text="s ${i}" \
        halignment=2 valignment=2 font-desc="Sans 40" \
    ! timeoverlay font-desc="Sans 40" \
    ! clockoverlay time-format="%S" font-desc="Sans 240" \
    ! video/x-raw, width=300, height=200 \
    ! gdppay \
    ! tcpclientsink port=3000 \
  & srcpid[i]=$!
done

read -p "press enter to kill all the processes:"

kill $srvpid $uipid 
for i in 1 2 3 4 5 6; do kill ${srcpid[i]}; done


