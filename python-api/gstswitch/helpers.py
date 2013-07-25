from testsource import VideoSrc, Preview

__all__ = ["TestSources", "PreviewSinks"]


class TestSources(object):
    """A Controller of test sources feeding into the
    gst-switch-srv"""

    def __init__(self, video_port):
        super(TestSources, self).__init__()
        self.running_tests = []
        self.video_port = video_port

    def new_test_video(self,
                       width=300,
                       height=200,
                       pattern=None,
                       timeoverlay=False,
                       clockoverlay=False):
        """Start a new test source
        """
        print 'Adding new test video source'
        testsrc = VideoSrc(
            self.video_port,
            width,
            height,
            pattern,
            timeoverlay,
            clockoverlay)
        if testsrc is None:
            pass
        self.running_tests.append(testsrc)

    def get_test_video(self):
        """Returns a list of processes acting as test inputs
        """
        i = 0
        for test in self.running_tests:
            print i, "pattern:", test.pattern
            i += 1
        return self.running_tests

    def terminate_index(self, index):
        """
        """
        testsrc = self.running_tests[index]
        print 'End source with pattern %s' % (str(testsrc.pattern))
        testsrc.end()
        self.running_tests.remove(self.running_tests[index])

    def terminate(self):
        """
        """
        print 'TESTS:', self.running_tests
        for test in range(len(self.running_tests)):
            self.terminate_index(0)


class PreviewSinks(object):
    """docstring for PreviewSinks
    """
    # TODO set preview port from dbus call

    def __init__(self, preview_port=3001):
        super(PreviewSinks, self).__init__()
        self.preview_port = preview_port

    def run(self):
        """Run the Preview Sink"""
        self.preview = Preview(self.preview_port)
        self.preview.run()
        print 'start preview'

    def terminate(self):
        """End/Terminate the Preview Sink"""
        try:
            self.preview.end()
            print 'end preview'
        except:
            pass
