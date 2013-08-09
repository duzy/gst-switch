import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.helpers import TestSources, PreviewSinks
from gstswitch.exception import RangeError, InvalidIndexError
import pytest
import gstswitch.testsource


class TestTestSourcesVideoPort(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                TestSources(video_port=test) 

    def test_range(self):
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                TestSources(video_port=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                TestSources(video_port=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = TestSources(video_port=test)
            assert src.video_port == test

class TestTestSources(object):

    class MockVideoSrc(object):

        def __init__(
            self,
            port,
            width=300,
            height=200,
            pattern=None,
            timeoverlay=False,
            clockoverlay=False):
            pass

        def run(self):
            pass


    def test_new_test_video(self, monkeypatch):
        test = TestSources(video_port=3000)
        monkeypatch.setattr(gstswitch.testsource, 'VideoSrc', self.MockVideoSrc)
        test.new_test_video()
        assert test.running_tests[0] is not None
        assert len(test.running_tests) != 0

    class MockTest(object):

            def __init__(self, pattern):
                self.pattern = pattern

            def end(self):
                pass

    def test_get_test_video(self):

        test = TestSources(video_port=3000)
        test.running_tests = [self.MockTest(1), self.MockTest(2), self.MockTest(3), self.MockTest(19)]
        test.get_test_video()

    def test_terminate_index_error(self):
        testsrc = TestSources(video_port=3000)
        testsrc.running_tests = [self.MockTest(1), self.MockTest(2), self.MockTest(3), self.MockTest(19)]
        tests = [-100, 20, 1e10, "hi", [1, 2, 3]]
        for test in tests:
            with pytest.raises(InvalidIndexError):
                testsrc.terminate_index(test)

    def test_terminate_index_normal(self):
        test = TestSources(video_port=3000)
        test.running_tests = [self.MockTest(1), self.MockTest(2), self.MockTest(3), self.MockTest(19)]
        test.terminate_index(0)


    def test_terminate1(self):
        test = TestSources(video_port=3000)
        test.running_tests = [self.MockTest(1), self.MockTest(2), self.MockTest(3), self.MockTest(19)]
        test.terminate()

    def test_terminate2(self):
        test = TestSources(video_port=3000)
        test.running_tests = []
        test.terminate()


class TestPreviewSinksPreviewPort(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                PreviewSinks(preview_port=test) 

    def test_range(self):
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                PreviewSinks(preview_port=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                PreviewSinks(preview_port=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = PreviewSinks(preview_port=test)
            assert src.preview_port == test



class TestPreviewSinks(object):

    class MockPreview(object):

        def __init__(self, preview_port=3001):
            pass

        def run(self):
            pass

        def end(self):
            pass

    def test_run(self, monkeypatch):
        preview = PreviewSinks()
        monkeypatch.setattr(gstswitch.testsource, 'Preview', self.MockPreview)
        preview.run()
        assert preview.preview is not None

    def test_terminate_fail(self):
        preview = PreviewSinks()
        with pytest.raises(AttributeError):
            preview.terminate()

    def test_terminate_normal(self, monkeypatch):
        preview = PreviewSinks()
        preview.preview = self.MockPreview()
        preview.terminate()
        assert preview.preview is None
