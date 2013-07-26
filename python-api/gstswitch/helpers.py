from testsource import VideoSrc, Preview

__all__ = ["TestSources", "PreviewSinks"]


class TestSources(object):
    """A Controller of test sources feeding into the
    gst-switch-srv
    :param width: The width of the output video
    :param height: The height of the output video
    :param pattern: The videotestsrc pattern of the output video
    :param timeoverlay: True to enable a running time over video
    :param clockoverlay: True to enable current clock time over video
    """

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
        """Start a new test video
        :param port: The port of where the TCP stream will be sent
        Should be same as video port of gst-switch-src
        :param width: The width of the output video
        :param height: The height of the output video
        :param pattern: The videotestsrc pattern of the output video
        :param timeoverlay: True to enable a running time over video
        :param clockoverlay: True to enable current clock time over video
        """
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
        """Returns a list of processes acting as video test sources running
        :returns: A list containing all video test sources running
        """
        i = 0
        for test in self.running_tests:
            print i, "pattern:", test.pattern
            i += 1
        return self.running_tests

    def terminate_index(self, index):
        """Terminate video test source specified by index
        :param index: The index of the video source to terminate
        Use get_test_video for finding the index
        """
        testsrc = self.running_tests[index]
        print 'End source with pattern %s' % (str(testsrc.pattern))
        testsrc.end()
        self.running_tests.remove(self.running_tests[index])

    def terminate(self):
        """Terminate all test video sources
        """
        print 'TESTS:', self.running_tests
        for i in range(len(self.running_tests)):
            self.terminate_index(0)


class PreviewSinks(object):
    """A Controller for preview sinks to preview ports of gst-switch-srv
    :param preview_port: The preview port to get the preview
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
            # TODO: Find which error may occur
        except OSError:
            raise
