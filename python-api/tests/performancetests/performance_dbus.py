"""
Performance/Torture test for DBus methods
"""


import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import TestSources, PreviewSinks
from gstswitch.controller import Controller
import time

PATH = '/usr/local/bin/'



def get_compose_port(num):
        """Test get_compose_port"""
        video_port = 8000
        serv = Server(path=PATH, video_port=video_port)
        try:
            serv.run()
            sources = TestSources(video_port=video_port)
            sources.new_test_video()
            
            expected_res = [video_port + 1] * num

            controller = Controller()
            controller.establish_connection()
            res = []
            for _ in range(num):
                res.append(controller.get_compose_port())

            assert expected_res == res
        finally:
            if serv.proc:
                    poll = serv.proc.poll()
                    if poll == -11:
                        print "SEGMENTATION FAULT OCCURRED"
                    print "ERROR CODE - {0}".format(poll)
                    serv.terminate(1)
                    log = open('server.log')
                    print log.read()


class TestGetComposePort100(object):
    """Performance test for get_compose_port method - 100 times"""

    NUMBER_TEST_RUNS = 100

    def test(self):
        get_compose_port(self.NUMBER_TEST_RUNS)


class TestGetComposePort200(object):
    """Performance test for get_compose_port method - 200 times"""

    NUMBER_TEST_RUNS = 200

    def test(self):
        get_compose_port(self.NUMBER_TEST_RUNS)


class TestGetComposePort300(object):
    """Performance test for get_compose_port method - 300 times"""

    NUMBER_TEST_RUNS = 300

    def test(self):
        get_compose_port(self.NUMBER_TEST_RUNS)
    

class TestGetComposePort400(object):
    """Performance test for get_compose_port method - 400 times"""

    NUMBER_TEST_RUNS = 400

    def test(self):
        get_compose_port(self.NUMBER_TEST_RUNS)


class TestGetComposePort500(object):
    """Performance test for get_compose_port method - 500 times"""

    NUMBER_TEST_RUNS = 500

    def test(self):
        get_compose_port(self.NUMBER_TEST_RUNS)


def get_encode_port(num):
        """Test get_encode_port"""
        video_port = 3000
        serv = Server(path=PATH, video_port=video_port)
        try:
            serv.run()
            sources = TestSources(video_port=video_port)
            sources.new_test_video()
            
            expected_res = [video_port + 2] * num

            controller = Controller()
            controller.establish_connection()
            res = []
            for _ in range(num):
                res.append(controller.get_encode_port())

            assert expected_res == res
        finally:
            if serv.proc:
                    poll = serv.proc.poll()
                    if poll == -11:
                        print "SEGMENTATION FAULT OCCURRED"
                    print "ERROR CODE - {0}".format(poll)
                    serv.terminate(1)
                    log = open('server.log')
                    print log.read()


class TestGetEncodePort100(object):
    """Performance test for get_encode_port method - 100 times"""

    NUMBER_TEST_RUNS = 100

    def test(self):
        get_encode_port(self.NUMBER_TEST_RUNS)


class TestGetEncodePort200(object):
    """Performance test for get_encode_port method - 200 times"""

    NUMBER_TEST_RUNS = 200

    def test(self):
        get_encode_port(self.NUMBER_TEST_RUNS)


class TestGetEncodePort300(object):
    """Performance test for get_encode_port method - 300 times"""

    NUMBER_TEST_RUNS = 300

    def test(self):
        get_encode_port(self.NUMBER_TEST_RUNS)
    

class TestGetEncodePort400(object):
    """Performance test for get_encode_port method - 400 times"""

    NUMBER_TEST_RUNS = 400

    def test(self):
        get_encode_port(self.NUMBER_TEST_RUNS)


class TestGetEncodePort500(object):
    """Performance test for get_encode_port method - 500 times"""

    NUMBER_TEST_RUNS = 500

    def test(self):
        get_encode_port(self.NUMBER_TEST_RUNS)



def get_audio_port(num):
        """Test get_audio_port"""
        audio_port = 8000
        serv = Server(path=PATH, audio_port=audio_port)
        try:
            serv.run()
            sources = TestSources(audio_port=audio_port)
            sources.new_test_audio()
            
            expected_res = [3003] * num

            controller = Controller()
            controller.establish_connection()
            res = []
            for _ in range(num):
                res.append(controller.get_audio_port())

            assert expected_res == res

        finally:
            if serv.proc:
                    poll = serv.proc.poll()
                    if poll == -11:
                        print "SEGMENTATION FAULT OCCURRED"
                    print "ERROR CODE - {0}".format(poll)
                    serv.terminate(1)
                    log = open('server.log')
                    print log.read()


class TestGetAudioPort100(object):
    """Performance test for get_audio_port method - 100 times"""

    NUMBER_TEST_RUNS = 100

    def test(self):
        get_audio_port(self.NUMBER_TEST_RUNS)


class TestGetAudioPort200(object):
    """Performance test for get_audio_port method - 200 times"""

    NUMBER_TEST_RUNS = 200

    def test(self):
        get_audio_port(self.NUMBER_TEST_RUNS)


class TestGetAudioPort300(object):
    """Performance test for get_audio_port method - 300 times"""

    NUMBER_TEST_RUNS = 300

    def test(self):
        get_audio_port(self.NUMBER_TEST_RUNS)
    

class TestGetAudioPort400(object):
    """Performance test for get_audio_port method - 400 times"""

    NUMBER_TEST_RUNS = 400

    def test(self):
        get_audio_port(self.NUMBER_TEST_RUNS)


class TestGetAudioPort500(object):
    """Performance test for get_audio_port method - 500 times"""

    NUMBER_TEST_RUNS = 500

    def test(self):
        get_audio_port(self.NUMBER_TEST_RUNS)

