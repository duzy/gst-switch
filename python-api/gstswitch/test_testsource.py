from exception import RangeError
from testsource import Preview, VideoSrc
import pytest
from mock import Mock, patch


class TestVideoSrcPort(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=test) 

    def test_range(self):
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                VideoSrc(port=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = VideoSrc(port=test)
            assert src.port == test

class TestVideoSrcWidth(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, width=test)

    def test_non_positive(self):
        tests = ['-1.111', -1.111, 0, -1e10]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, width=test)

    def test_invalid(self):
        tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, width=test)

    def test_normal(self):
        tests = [  1e6, 300, '200']
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, width=test)
            assert src.width == test


class TestVideoSrcHeight(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, height=test)

    def test_non_positive(self):
        tests = ['-1.111', -1.111, 0, -1e10]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, height=test)

    def test_invalid(self):
        tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, height=test)

    def test_normal(self):
        tests = [1e6, 300, '200']
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, height=test)
            assert src.height == test


class TestVideoSrcPattern(object):

    def test_range(self):
        tests = [-100, 1e7, 65536, -1, 20]
        port = 1000
        for test in tests:
            with pytest.raises(RangeError):
                VideoSrc(port=port, pattern=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, pattern=test)

    def test_normal(self):
        tests = [1, 0, 19, 10]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, pattern=test)
            assert src.pattern == str(test)


class TestVideoSrcTimeOverlay(object):

    def test_fail(self):
        tests = ['', 1234, 'hi', [1, 2], {1: 2}, None, 0, []]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, timeoverlay=test)

    def test_normal(self):
        tests = [True, False]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, timeoverlay=test)
            assert src.timeoverlay == test



class TestVideoSrcClockOverlay(object):

    def test_fail(self):
        tests = ['', 1234, 'hi', [1, 2], {1: 2}, None, 0, []]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, clockoverlay=test)

    def test_normal(self):
        tests = [True, False]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, clockoverlay=test)
            assert src.clockoverlay == test

class MockPipeline(object):

    def play(self):
        pass

    def pause(self):
        pass

    def disable(self):
        pass


class TestVideoSrcPlay(object):

    def test_run(self):
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.run()

    def test_pause(self):
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.pause()

    def test_end(self):
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.end()


class TestPreviewPort(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                Preview(port=test) 

    def test_range(self):
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                Preview(port=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                Preview(port=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = Preview(port=test)
            assert src.preview_port == test

class TestPreviewPlay(object):

    def test_run(self):
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.run()

    def test_pause(self):
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.pause()

    def test_end(self):
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.end()