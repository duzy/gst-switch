#!/usr/bin/env python

# WARNING:
# THIS FILE IS ONLY FOR MY PERSONAL TESTING PURPOSES.
# IT IS NOT MADE TO AND SHOULD NOT BE TESTING THE ENTIRE API

from gstswitch.server import Server
from gstswitch.helpers import *
from gstswitch.controller import Controller
import time

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/stage/bin/'
# path = '/usr/local/bin/'
s = Server(path)
try:
    s.run()  # launches the server default parameters
    port = s.video_port
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
    tests_get = [controller.get_compose_port,
                 controller.get_encode_port,
                 controller.get_audio_port,
                 controller.get_preview_ports,
                 ]

    test_set_composite_mode = controller.set_composite_mode

    # testing random 10 modes
    for x in tests_get:
        print x()
    time.sleep(1)
    modes = [3, 2, 1, 0, 1, 2, 3, 0]
    for mode in modes:
        print 'composite mode=', mode
        test_set_composite_mode(mode)
        time.sleep(1)

    sources.terminate()
    s.terminate()

finally:
    if s.proc:
        s.kill()
