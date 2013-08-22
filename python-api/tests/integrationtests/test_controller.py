import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
from gstswitch.controller import Controller
import time
import datetime

from compare import CompareVideo

import subprocess

PATH = '/home/hyades/gst/stage/bin/'

class TestEstablishConnection(object):

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



class TestGetComposePort(object):

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


class TestGetEncodePort(object):

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


class TestGetAudioPortVideoFirst(object):

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


class TestGetAudioPortAudioFirst(object):

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
                for j in range(i):
                    sources.new_test_video()

                res.append(self.get_audio_port())

                sources.terminate_video()
                sources.terminate_audio()
                s.terminate()
            finally:
                if s.proc:
                    s.terminate()

        at = [ tuple(i) for i in expected_result]
        bt = [ tuple(i) for i in res]
        assert set(at) == set(bt)


class TestGetPreviewPorts(object):

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


class VideoFileSink(object):
    """Sink the video to a file
    """
    def __init__(self, path, port, filename):
        cmd = "{0}/gst-launch-1.0 tcpclientsrc port={1} ! gdpdepay !  jpegenc ! avimux ! filesink location={2}".format(path, port, filename)
        with open(os.devnull, 'w') as tempf:
            self.proc = subprocess.Popen(
            cmd.split(),
            stdout=tempf,
            stderr=tempf,
            bufsize=-1,
            shell=False)

    def terminate(self):
        self.proc.terminate()


class TestSetCompositeMode(object):

    NUM = 1
    FACTOR = 1

    def set_composite_mode(self, mode, generate_frames=False):

        for i in range(self.NUM):

            s = Server(path=PATH)
            try:
                s.run()
                
                preview = PreviewSinks()
                preview.run()

                out_file = 'output-{0}.data'.format(mode)
                video_sink = VideoFileSink(PATH, s.video_port+1, out_file)

                sources = TestSources(video_port=3000)
                sources.new_test_video(pattern=4)
                sources.new_test_video(pattern=5)
                
                time.sleep(3)
                # expected_result = [mode != 3] * self.FACTOR
                # print mode, expected_result
                controller = Controller()
                res = controller.set_composite_mode(mode)
                print res
                time.sleep(3)
                video_sink.terminate()
                preview.terminate()
                sources.terminate_video()
                s.terminate()
                if not generate_frames:
                    if mode == 3:
                        assert res is False
                    else:
                        assert res is True
                    self.prev_mode = mode
                    assert self.verify_output(mode, out_file) == True
                # assert expected_result == res

            finally:
                if s.proc:
                    s.terminate()
                pass

    def verify_output(self, mode, video):
        test = 'composite_mode_{0}'.format(mode)
        cmpr = CompareVideo(test, video)
        res1, res2 = cmpr.compare()
        print "RESULTS", res1, res2
        # TODO Experimental Value
        if res1 == 0 and res2 == 0:
            return True
        return False

    def test_set_composite_mode(self):
        for i in range(4):
            self.set_composite_mode(i)


class TestNewRecord(object):

    NUM = 1
    FACTOR = 1
    def new_record(self):
        r = []
        controller = Controller()
        for i in range(self.NUM * self.FACTOR):
            r.append(controller.new_record())
        return r

    def test_new_record(self):

        for i in range(self.NUM):
            s = Server(path=PATH, record_file="test.data")
            try:
                s.run()

                sources = TestSources(video_port=3000)
                sources.new_test_video()
                sources.new_test_video()

                curr_time = datetime.datetime.now()
                alt_curr_time = curr_time + datetime.timedelta(0,1) 
                time_str =  curr_time.strftime('%Y-%m-%d %H%M%S')
                alt_time_str = alt_curr_time.strftime('%Y-%m-%d %H%M%S')
                test_filename = "test {0}.data".format(time_str)
                alt_test_filename = "test {0}.data".format(alt_time_str)

                res = self.new_record()
                print res
                sources.terminate_video()
                s.terminate()
                # print ((os.path.exists(test_filename)) or (os.path.exists(alt_test_filename)))
                assert ((os.path.exists(test_filename)) or (os.path.exists(alt_test_filename))) == True
            finally:
                if s.proc:
                    s.terminate()


class TestAdjustPIP(object):
    NUM = 1
    FACTOR = 1
    
    def adjust_pip(self, dx, dy, dw, dh, index, generate_frames=False):
        
        for i in range(self.NUM):
            s = Server(path=PATH)
            try:
                s.run()
                sources = TestSources(video_port=3000)
                preview = PreviewSinks()
                preview.run()
                out_file = "output-{0}.data".format(index)
                video_sink = VideoFileSink(PATH, 3001, out_file)
                sources.new_test_video(pattern=4)
                sources.new_test_video(pattern=5)
                controller = Controller()
                controller.set_composite_mode(1)
                time.sleep(3)
                res = controller.adjust_pip(dx, dy, dw, dh)
                time.sleep(3)
                sources.terminate_video()
                preview.terminate()
                video_sink.terminate()
                s.terminate()
                if not generate_frames:
                    assert res is not None
                    assert self.verify_output(index, out_file) == True


            finally:
                if s.proc:
                    s.terminate()

    def verify_output(self, index, video):
        test = 'adjust_pip_{0}'.format(index)
        cmpr = CompareVideo(test, video)
        res1, res2 = cmpr.compare()
        print "RESULTS", res1, res2
        # TODO Experimental Value
        if res1 == 0 and res2 == 0:
            return True
        return False

    def test_adjust_pip(self):
        d = [
                [50, 75, 0, 0],
            ]
        for i in range(4,5):
            self.adjust_pip(d[i-4][0], d[i-4][1], d[i-4][2], d[i-4][3], i)

