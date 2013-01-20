#!/bin/bash
./tests/test-switch-server \
    --disable-test-controller \
    --disable-test-video \
    --disable-test-audio \
    --disable-test-ui-integration \
    --disable-test-fuzz-ui \
    --test-external-server \
    --test-external-ui \
    --disable-test-switching \

#    --disable-test-random-connection \
