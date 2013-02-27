#!/bin/bash

coproc SERVER( \
    ./tools/gst-switch-srv -v \
    --gst-debug-no-color \
    --record=timestamped.data \
    )
sleep 2 && coproc CLIENT( \
    ./tools/gst-switch-ui \
    --gst-debug-no-color \
    )
sleep 1 && coproc FEED( \
    ./tests/feed-timestamped-video.sh \
    )

echo "server: $SERVER_PID"
echo "client: $CLIENT_PID"

wait $CLIENT_PID
kill $SERVER_PID
