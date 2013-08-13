import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time
import glob


PATH = '/home/hyades/gst/stage/bin/'

class TestServerStartStop(object):

    def __init__(self, num):
        s = Server(path=PATH)
        try:
            s.run()
            pid = s.pid
            assert pid > 0
            s.terminate()
            assert s.proc is None
        except OSError:
            if s.proc:
                s.kill()
            assert s.proc is None
            print "Test Failed"


