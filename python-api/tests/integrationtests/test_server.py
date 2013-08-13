import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time
import glob


PATH = '/home/hyades/gst/stage/bin/'


class TestServerStartStop(object):

    NUM = 5
    
    def startstop(self):
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

    def test_start_stop(self):
        for i in range(self.NUM):
            self.startstop()
        # remove all .data files generated
        for f1 in glob.glob(os.getcwd()+'/*.data'):
            os.remove(f1)