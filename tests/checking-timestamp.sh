#!/bin/bash

coproc SERVER( ./tools/gst-switch-srv --record=timestamped.data )
sleep 2 && coproc CLIENT( ./tools/gst-switch-ui )
sleep 1 && coproc FEED( ./tests/feed-timestamped-video.sh )

echo "server: $SERVER_PID"
echo "client: $CLIENT_PID"

wait $CLIENT_PID
kill $SERVER_PID
