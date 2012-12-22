function goto_gstreamer_stage_directory() {
    cd /store/open/gstreamer/stage
}

function launch() {
    goto_gstreamer_stage_directory
    ./bin/gst-launch-1.0 $@
    cd - > /dev/null
}

function check_file() {
    echo "=================================================="
    echo "GOT:"
    cat $1
    echo "--------------------------------------------------"
    echo "EXPECT:"
    echo -n "$2"
    echo "=================================================="
}
