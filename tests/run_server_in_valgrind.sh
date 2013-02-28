#!/bin/bash
/usr/bin/valgrind \
    "--show-reachable=yes" \
    "--track-origins=yes" \
    "--track-fds=yes" \
    "--tool=memcheck" \
    "--leak-check=full" \
    "--leak-resolution=high" \
    "--num-callers=20" \
    "--log-file=test-switch-server-valgrind.log" \
    "--suppressions=../gstreamer/common/gst.supp" \
    "--suppressions=../gst-plugins-base/tests/check/gst-plugins-base.supp" \
    "--suppressions=../gst-plugins-good/tests/check/gst-plugins-good.supp" \
    "--suppressions=../gst-plugins-bad/tests/check/gst-plugins-bad.supp" \
    "--suppressions=../gst-plugins-ugly/tests/check/gst-plugins-ugly.supp" \
    "--suppressions=tests/faac.supp" \
    "./tools/gst-switch-srv" "-v" \
    "--gst-debug-no-color" \

#    "--record=test-recording.data"
