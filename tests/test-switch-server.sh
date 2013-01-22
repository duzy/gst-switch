#!/bin/bash
EXT=""
EXT="$EXT --test-external-server"
EXT="$EXT --test-external-ui"

./tests/test-switch-server $EXT \
    --disable-test-controller \
    --disable-test-video \
    --disable-test-ui-integration \
    --disable-test-fuzz-ui \
    --disable-test-random-connection \
    --disable-test-audio \

#    --disable-test-switching \
#    --disable-test-checking-timestamps \
