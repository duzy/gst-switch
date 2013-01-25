#!/bin/bash
#EXT="--gst-debug-no-color"
#EXT="$EXT --test-external-server"
#EXT="$EXT --test-external-ui"

./tests/test-switch-server $EXT \
    --enable-test-controller \

#    --enable-test-video \
#    --enable-test-ui-integration \
#    --enable-test-fuzz-ui \
#    --enable-test-random-connection \
#    --enable-test-audio \
#    --enable-test-switching \
#    --enable-test-checking-timestamps \
