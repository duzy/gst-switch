import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time


PATH = '/home/hyades/gst/stage/bin/'


class TestTestSourcesPreviews(object):

    NUM = 10

    def add_video_sources(self, num, video_port):
        sources = TestSources(video_port=video_port)
        for i in range(num):
            sources.new_test_video(pattern=18)
        # print "done adding" + str(num)
        time.sleep(2)
        sources.terminate_video()
        time.sleep(2)

    def test_video_sources(self):
        video_port = 3000
        s = Server(PATH, video_port=video_port)
        try:
            s.run()
            preview = PreviewSinks()
            preview.run()
            for i in range(self.NUM):
                self.add_video_sources(i*10+1, video_port)
            preview.terminate()
            s.terminate()
        finally:
            if s.proc:
                s.kill()

    def add_audio_sources(self, num, audio_port):
        sources = TestSources(audio_port=audio_port)
        for i in range(num):
            sources.new_test_audio(port=audio_port, wave=10)
        # print "done adding" + str(num)
        # print sources.get_test_audio()
        time.sleep(5)
        sources.terminate_audio()
        time.sleep(5)

    def test_audio_sources(self):
        audio_port=4000
        s = Server(PATH, audio_port=audio_port)
        try:
            s.run()
            preview = PreviewSinks()
            preview.run()
            for i in range(self.NUM):
                self.add_audio_sources(i*10+1, audio_port)

            preview.terminate()
            s.terminate()
        finally:
            if s.proc:
                s.kill()