#!/bin/bash
#EXT="--gst-debug-no-color"
EXT="$EXT --test-external-server"
EXT="$EXT --test-external-ui"

./tests/test-switch-server $EXT \
    --enable-test-controller \

#    --enable-test-composite-mode \
#    --enable-test-random \
#    --enable-test-composite-mode \
#    --enable-test-switching \
#    --enable-test-composite-mode \
#    --enable-test-controller \
#    --enable-test-video \
#    --enable-test-ui \
#    --enable-test-fuzz \
#    --enable-test-audio \
#    --enable-test-checking-timestamps \
