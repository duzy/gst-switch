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
        """Test get_compose_port - num times"""
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


class TestGetComposePort(object):
    """Performance test for get_compose_port method - 100 times"""

    def test_100(self):
        """Performance test for get_compose_port method"""
        number_test_runs = 100
        get_compose_port(number_test_runs)


    def test_200(self):
        """Performance test for get_compose_port method"""
        number_test_runs = 200
        get_compose_port(number_test_runs)


    def test_300(self):
        """Performance test for get_compose_port method"""
        number_test_runs = 300
        get_compose_port(number_test_runs)
    

    def test_400(self):
        """Performance test for get_compose_port method"""
        number_test_runs = 400
        get_compose_port(number_test_runs)

    def test_500(self):
        """Performance test for get_compose_port method"""
        number_test_runs = 500
        get_compose_port(number_test_runs)


def get_encode_port(num):
        """Test get_encode_port - num times"""
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


class TestGetEncodePort(object):
    """Performance test for get_encode_port method"""

    def test_100(self):
        """Performance test for get_encode_port method - 100 times"""
        number_test_runs = 100
        get_encode_port(number_test_runs)

    def test_200(self):
        """Performance test for get_encode_port method - 200 times"""
        number_test_runs = 200
        get_encode_port(number_test_runs)

    def test_300(self):
        """Performance test for get_encode_port method - 300 times"""
        number_test_runs = 300
        get_encode_port(number_test_runs)

    def test_400(self):
        """Performance test for get_encode_port method - 400 times"""
        number_test_runs = 400
        get_encode_port(number_test_runs)

    def test_500(self):
        """Performance test for get_encode_port method - 500 times"""
        number_test_runs = 500
        get_encode_port(number_test_runs)



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


class TestGetAudioPort(object):
 
    def test_100(self):
        """Performance test for get_audio_port method - 100 times"""
        number_test_runs = 100
        get_audio_port(number_test_runs)

    
    def test_200(self):
        """Performance test for get_audio_port method - 200 times"""
        number_test_runs = 200
        get_audio_port(number_test_runs)

    
    def test_300(self):
        """Performance test for get_audio_port method - 300 times"""
        number_test_runs = 300
        get_audio_port(number_test_runs)
    
    
    def test_400(self):
        """Performance test for get_audio_port method - 400 times"""
        number_test_runs = 400
        get_audio_port(number_test_runs)

    
    def test_500(self):
        """Performance test for get_audio_port method - 500 times"""
        number_test_runs = 500
        get_audio_port(number_test_runs)


def get_preview_ports(num):
    """Get Preview Ports when num number of sources are added"""
    video_port = 3000
    serv = Server(path=PATH, video_port=video_port)
    try:
        serv.run()
        sources = TestSources(video_port=video_port)
        sources.new_test_video()
        
        expected_res = [ i for i in range(3003, 3003 + num)]

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


class TestGetPreviewPorts(object):

    def test_100():
        """Test when 100 sources are added"""
        number_souces = 100
        get_preview_ports(number_souces)
