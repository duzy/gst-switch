import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
from gstswitch.controller import Controller
import time

PATH = '/home/hyades/gst/stage/bin/'

class sTestEstablishConnection(object):

    NUM = 1
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
            s.terminate()
        finally:
            if s.proc:
                s.terminate()



class sTestGetComposePort(object):

    NUM = 1
    FACTOR = 1
    def get_compose_port(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM*self.FACTOR):
            r.append(controller.get_compose_port())
        return r

    def test_compose_ports(self):
        res = []
        expected_result = []
        for i in range(self.NUM):
            video_port = (i+1)*1000
            expected_result.append([video_port+1]*self.NUM*self.FACTOR)
            s = Server(path=PATH, video_port=video_port)
            try:
                s.run()
                sources = TestSources(video_port=video_port)
                sources.new_test_video()
                sources.new_test_video()

                res.append(self.get_compose_port())
                sources.terminate_video()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()
        
        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)


class sTestGetEncodePort(object):

    NUM = 1
    FACTOR = 1
    def get_encode_port(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM*self.FACTOR):
            r.append(controller.get_encode_port())
        return r

    def test_encode_ports(self):
        res = []
        expected_result = []
        for i in range(self.NUM):
            video_port = (i+1)*1000
            expected_result.append([video_port+2]*self.NUM*self.FACTOR)
            s = Server(path=PATH, video_port=video_port)
            try:
                s.run()
                sources = TestSources(video_port=video_port)
                sources.new_test_video()
                sources.new_test_video()

                res.append(self.get_encode_port())
                sources.terminate_video()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()
        
        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)


class sTestGetAudioPortVideoFirst(object):

    NUM = 1
    FACTOR = 1
    def get_audio_port(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM*self.FACTOR):
            r.append(controller.get_audio_port())
        return r

    def test_audio_ports(self):
        res = []
        expected_result = []
        for i in range(1, self.NUM+1):
            audio_port = (i+10)*1000
            expected_result.append([3003 + i] * self.NUM * self.FACTOR)
            s = Server(path=PATH, video_port=3000, audio_port=audio_port)
            try:
                s.run()
                sources = TestSources(video_port=3000, audio_port=audio_port)
                for j in range(i):
                    sources.new_test_video()
                sources.new_test_audio()
                sources.new_test_audio()
                res.append(self.get_audio_port())
                sources.terminate_video()
                sources.terminate_audio()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()
        # print res
        # print expected_result
        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)


class sTestGetAudioPortAudioFirst(object):

    NUM = 1
    FACTOR = 1
    def get_audio_port(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM * self.FACTOR):
            r.append(controller.get_audio_port())
        return r

    def test_audio_ports(self):
        res = []
        expected_result = []
        for i in range(1, self.NUM+1):
            audio_port = (i+10)*1000
            expected_result.append([3003] * self.NUM * self.FACTOR)
            s = Server(path=PATH, video_port=3000, audio_port=audio_port)
            try:
                s.run()
                sources = TestSources(video_port=3000, audio_port=audio_port)
                
                sources.new_test_audio()
                sources.new_test_audio()
                for j in range(i):
                    sources.new_test_video()
                res.append(self.get_audio_port())
                sources.terminate_video()
                sources.terminate_audio()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()
        # print res
        # print expected_result
        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)


class sTestGetPreviewPorts(object):

    NUM = 1
    FACTOR = 1
    def get_preview_ports(self):
        r = []
        controller = Controller()
        controller.establish_connection()
        for i in range(self.NUM * self.FACTOR):
            r.append(controller.get_preview_ports())
        return r

    def test_get_preview_ports(self):
        
        for  i in range(self.NUM):
            s = Server(path=PATH)
            try:
                s.run()
                sources = TestSources(video_port=3000, audio_port=4000)
                for i in range(self.NUM):
                    sources.new_test_audio()
                    sources.new_test_video()
                # print map(tuple, [[x for x in range(3003, 3003 + self.NUM * 10)]]*self.NUM*self.FACTOR), '\n'
                expected_result = map(tuple, [[x for x in range(3003, 3004 + self.NUM)]]*self.NUM*self.FACTOR)
                res = map(tuple, self.get_preview_ports())
                print '\n', res, '\n'
                print expected_result
                assert set(expected_result) == set(res)
                sources.terminate_video()
                sources.terminate_audio()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()


class TestSetCompositeMode(object):

    NUM = 1
    FACTOR = 1
    def set_composite_mode(self, mode):
        r = []
        controller = Controller()
        controller.establish_connection()
        time.sleep(1)
        # controller.get_compose_port()
        for i in range(self.FACTOR):
            print "\nchange composite mode - ", mode
            r.append(controller.set_composite_mode(mode))
        return r

    def driver_set_composite_mode(self, mode):
        for i in range(self.NUM):

            s = Server(path=PATH)
            try:
                s.run()
                
                preview = PreviewSinks()
                preview.run()

                sources = TestSources(video_port=3000)
                sources.new_test_video()
                sources.new_test_video()
                
                time.sleep(5)
                # expected_result = [mode != 3] * self.FACTOR
                # print mode, expected_result
                res = self.set_composite_mode(mode)
                print res
                time.sleep(5)
                preview.terminate()
                sources.terminate_video()
                s.terminate()
                # assert expected_result == res

            finally:
                if s.proc:
                    s.terminate()
                pass
    def test_set_composite_mode(self):
        for i in range(1):
            # print "\n\nmode\n\n", i
            self.driver_set_composite_mode(i)

