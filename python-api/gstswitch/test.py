#!/usr/bin/env python
from gstswitch import *
from helpers import TestSources, PreviewSinks

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/master/gstreamer/tools/'
s = Server(path)
try:
    s.run()  # launches the server default parameters
    port = s.get_video_port()
    # connects a gstreamer module to view the output of the gst-switch-srv
    output = PreviewSinks()
    output.start()
    # adding two test video sources
    sources = TestSources(port)
    sources.new_test_video()
    sources.new_test_video(timeoverlay=True)
    # waiting till user ends the server
    raw_input()
    sources.terminate()
    output.terminate()
    s.terminate()
finally:
    s.kill()
