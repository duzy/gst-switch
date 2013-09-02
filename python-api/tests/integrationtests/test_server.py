import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *


# PATH = os.getenv("HOME") + '/gst/stage/bin/'
PATH = '/usr/local/bin/'

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
            s.terminate(1)
            assert s.proc is None
        finally:
                if s.proc:
                    poll = s.proc.poll()
                    print self.__class__
                    if poll == -11:
                        print "SEGMENTATION FAULT OCCURRED"
                    print "ERROR CODE - {0}".format(abs(poll))
                    s.terminate(1)
                    f = open('server.log')
                    print f.read()

    def test_start_stop(self):
        for i in range(self.NUM):
            self.startstop()
        # remove all .data files generated
        # for f1 in glob.glob(os.getcwd()+'/*.data'):
        #     os.remove(f1)
