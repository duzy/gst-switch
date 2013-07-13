#!/usr/bin/env python
from gstswitch import *
from helpers import TestSources, PreviewSinks
from controller import Controller
import time
import random

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/master/gstreamer/tools/'
s = Server(path)
get_res = ''
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
    tests_get = [controller.get_compose_port,
                 controller.get_encode_port,
                 controller.get_audio_port,
                 controller.get_preview_ports,
                 ]

    test_set_composite_mode = controller.set_composite_mode

    # testing random 10 modes
    for x in tests_get:
        get_res += str(x())
        get_res += '\n'
    time.sleep(2)
    count = 0
    modes = [0, 3, 3, 3, 1, 0, 3, 2, 2, 2, 0]
    for mode in modes:
        print 'composite mode=', mode
        get_res += str(test_set_composite_mode(mode))
        get_res += '\n'
        count += 1
        time.sleep(2)

    # test_set_composite_mode(0)
    # time.sleep(2)
    # channel = 1
    # while True:
    #     print 'encode mode=', channel
    #     test_set_encode_mode(channel % 2 + 1)
    #     channel += 1
    #     time.sleep(2)
    #     if channel == 5:
    #         break



    time.sleep(0.1)
    sources.terminate()
    output.terminate()
    s.terminate()

finally:
    s.kill()
    print 'get:result'
    print get_res
    print modes
