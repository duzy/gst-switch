import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
from gstswitch.controller import Controller
import time

PATH = '/home/hyades/gst/stage/bin/'

class TestEstablishConnection(object):

    NUM = 3
    # fails above 3
    def establish_connection(self):
        controller = Controller()
        controller.establish_connection()
        # print controller.connection
        assert controller.connection is not None

    def test_establish(self):
        s = Server(path=PATH)
        try:
            s.run()
            for i in range(self.NUM):
                print i
                self.establish_connection()
                time.sleep(2)
            s.terminate()
        finally:
            if s.proc:
                s.terminate()



class TestGetComposePort(object):

    NUM = 4
    def get_compose_port(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM*3):
            r.append(controller.get_compose_port())
        return r

    def test_compose_ports(self):
        res = []
        for i in range(self.NUM):
            video_port = (i+1)*1000
            s = Server(path=PATH, video_port=video_port)
            try:
                s.run()
                sources = TestSources(video_port=video_port)
                sources.new_test_video()
                sources.new_test_video()

                res.append(self.get_compose_port())
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()
        expected_result = [[1001]*self.NUM*3,
                          [2001]*self.NUM*3, 
                          [3001]*self.NUM*3,
                          [4001]*self.NUM*3]
        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)

                   
                




