#!/usr/bin/env python
from gstswitch import *
from time import sleep
# import subprocess

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/master/gstreamer/tools/'
# os.chdir(path)
print "starting server"
s = Server(path)
try:
    s.run()  # launches the server default parameters
    sleep(0.5)
    cmd = path
    # connects a gstreamer module to view the output of the gst-switch-srv
    s.start_preview()
    # adding two test video sources
    s.new_test_video()
    s.new_test_video(clockoverlay=True)
    # waiting till user ends the server
    raw_input()
    s.end()
finally:
    s.brute_end()
