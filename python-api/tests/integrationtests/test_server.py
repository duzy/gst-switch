import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time
import glob


PATH = os.getenv("HOME") + '/gst/stage/bin/'


class TestServerStartStop(object):
    """Test Starting and Stopping the Server
    Run Server and Stop multiple times
    """
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
                f = open('server.log')
                print f.read()

    def test_start_stop(self):
        for i in range(self.NUM):
            self.startstop()
        # remove all .data files generated
        # for f1 in glob.glob(os.getcwd()+'/*.data'):
        #     os.remove(f1)
