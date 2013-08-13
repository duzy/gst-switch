import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time

from multiprocessing import Pool

PATH = '/home/hyades/gst/stage/bin/'


class TestTestSourcesPreviews(object):

    NUM = 1

    def new_video(self, x):
        self.sources.new_test_video()

    def sourcepreview(self, num, video_port): 
        self.sources = TestSources(video_port)
        p = Pool(num)
        p.map(self.new_video, range(num))
        time.sleep(2)
        self.sources.terminate_video()

    def test_sources(self):
        video_port = 3000
        s = Server(PATH, video_port=video_port)
        try:
            s.run()
            preview = PreviewSinks()
            preview.run()
            for i in range(self.NUM):
                self.sourcepreview(i+10, video_port)
            preview.terminate()
            s.terminate()
        finally:
            if s.proc:
                s.kill()