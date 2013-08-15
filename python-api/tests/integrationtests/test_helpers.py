import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
import time

from multiprocessing import Pool

PATH = '/home/hyades/gst/stage/bin/'


class TestTestSourcesPreviews(object):

    NUM = 10

    def sourcepreview(self, num, video_port): 
        sources = TestSources(video_port)
        for i in range(num):
            sources.new_test_video(pattern=18)
        # print "done adding" + str(num)
        time.sleep(5)
        sources.terminate_video()
        time.sleep(5)

    def test_sources(self):
        video_port = 3000
        s = Server(PATH, video_port=video_port)
        try:
            s.run()
            preview = PreviewSinks()
            preview.run()
            for i in range(self.NUM):
                self.sourcepreview(i*10+1, video_port)
            preview.terminate()
            s.terminate()
        finally:
            if s.proc:
                s.kill()