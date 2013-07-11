#!/usr/bin/env python
from gstswitch import *
from helpers import TestSources, PreviewSinks
from controller import Controller
import time

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/master/gstreamer/tools/'
s = Server(path)
try:
    s.run()  # launches the server default parameters
    port = s.get_video_port()
    # connects a gstreamer module to view the output of the gst-switch-srv
    output = PreviewSinks()
    output.run()
    # adding two test video sources
    sources = TestSources(port)
    sources.new_test_video()
    sources.new_test_video(timeoverlay=True)
    # waiting till user ends the server
    controller = Controller()
    controller.establish_connection()
    res = []
    get_res = ''
    tests_get = [controller.get_compose_port, controller.get_encode_port, controller.get_audio_port,
                 controller.get_preview_ports]
    for x in tests_get:
        get_res += str(x())
        get_res += '\n'
    time.sleep(0.1)
    sources.terminate()
    output.terminate()
    s.terminate()
    print 'output'
    print get_res
finally:
    s.kill()
