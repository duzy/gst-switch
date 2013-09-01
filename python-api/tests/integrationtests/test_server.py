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
            s.terminate()
            assert s.proc is None
        finally:
                if s.proc:
                    poll = s.proc.poll()
                    s.terminate()
                    try:
                        if poll < 0:
                            error_type, ob, tb = sys.exc_info()
                            server_log = open('server.log').read()
                            error_msg = ob.message
                            custom_error = """
                                {0}
                                -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
                                gst-switch-srv log:
                                Error Code - {1} (http://tldp.org/LDP/abs/html/exitcodes.html)
                                {2}
                                """.format(error_msg, abs(poll), server_log)
                            raise ob.__class__(custom_error)
                    except:
                        pass

    def test_start_stop(self):
        for i in range(self.NUM):
            self.startstop()
        # remove all .data files generated
        # for f1 in glob.glob(os.getcwd()+'/*.data'):
        #     os.remove(f1)
