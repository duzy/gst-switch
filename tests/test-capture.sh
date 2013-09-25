BAD="/store/open/gst-plugins-bad/ext/speakertrack"
cd $BAD
echo "build: Entering directory \`$BAD'"
make install
echo "build: Leaving directory \`$BAD'"
cd -

make && (
    if false; then
	coproc SERVER ( \
	    ./tools/gst-switch-srv \
	    )
	sleep 1 && coproc UI ( \
	    ./tools/gst-switch-ui \
	    )
    fi
    sleep 1 && ./tools/gst-switch-cap
    sleep 1 && kill -9 $UI_PID
    sleep 1 && kill -9 $SERVER_PID
)
