import os
import sys
import signal
import subprocess
from gi.repository import Gio
from testsource import VideoSrc, Preview


class TestSources(object):
    """A Controller of test sources feeding into the
    gst-switch-srv"""

    def __init__(self, video_port):
        super(TestSources, self).__init__()
        self.TESTS = []
        self.set_video_port(video_port)

    def set_video_port(self, video_port):
        self.VIDEO_PORT = video_port

    def get_video_port(self):
        return self.VIDEO_PORT

    def new_test_video(self, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
        """Start a new test source
        """
        print 'Adding new test video source'
        testsrc = VideoSrc(self.get_video_port(), width, height, pattern, timeoverlay, clockoverlay)
        if testsrc is None:
            pass
        self.TESTS.append(testsrc)

    def get_test_video(self):
        """Returns a list of processes acting as test inputs
        """
        i = 0
        for x in self.TESTS:
            print i, "pattern:", x.pattern
            i += 1
        return self.TESTS

    def terminate_index(self, index):
        """
        """
        testsrc = self.TESTS[index]
        print 'End source with pattern %s' % (str(testsrc.get_pattern()))
        testsrc.end()
        self.TESTS.remove(self.TESTS[index])

    def terminate(self):
        """
        """
        print 'TESTS:', self.TESTS
        for x in range(len(self.TESTS)):
            self.terminate_index(0)


class PreviewSinks(object):
    """docstring for PreviewSinks
    """
    def __init__(self):
        super(PreviewSinks, self).__init__()
        self.PREVIEW_PORT = 3001
        # TODO set preview port from dbus call

    def run(self):
        self.preview = Preview(self.PREVIEW_PORT)
        self.preview.run()
        print 'start preview'

    def terminate(self):
        try:
            self.preview.end()
            print 'end preview'
        except:
            pass
