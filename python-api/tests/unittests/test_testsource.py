import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.exception import RangeError
from gstswitch.testsource import Preview, VideoSrc, BasePipeline, VideoPipeline, AudioPipeline, AudioSrc
import pytest
from mock import Mock, patch
from gi.repository import Gst


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



class TestBasePipeline(object):

    def test_play(self, monkeypatch):
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.play()

    def test_pause(self, monkeypatch):
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.pause()

    def test_disable(self, monkeypatch):
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.disable()


class TestVideoPipeline(object):

    def test_permuate_time_clock_1(self):
        VideoPipeline(port=3000, pattern=10, timeoverlay=False, clockoverlay=False)

    def test_permuate_time_clock_2(self):
        VideoPipeline(port=3000, pattern=10, timeoverlay=False, clockoverlay=True)

    def test_permuate_time_clock_3(self):
        VideoPipeline(port=3000, pattern=10, timeoverlay=True, clockoverlay=False)

    def test_permuate_time_clock_4(self):
        VideoPipeline(port=3000, pattern=10, timeoverlay=True, clockoverlay=True)


class TestAudioSrcPort(object):

    def test_blank(self):
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                AudioSrc(port=test) 

    def test_range(self):
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = AudioSrc(port=test)
            assert src.port == test


class TestAudioSrcFreq(object):

    def test_blank(self):
        tests = ['', None, [], {}, 0]
        port = 4000
        for test in tests:
            with pytest.raises(ValueError):
                AudioSrc(port=port, freq=test) 

    def test_range(self):
        tests = [-100,  -1]
        port = 4000
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=port, freq=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        port = 4000
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=port, freq=test)

    def test_normal(self):
        tests = [1, 65535, 1000]
        for test in tests:
            src = AudioSrc(port=4000, freq=test)
            assert src.freq == test


class TestAudioSrcWave(object):

    def test_range(self):
        tests = [-100,  -1, 13, 1e2, '1e10']
        port = 4000
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=port, wave=test)

    def test_invalid(self):
        tests = [[1, 2, 3, 4], {1: 2, 2: 3},]
        port = 4000
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=port, wave=test)

    def test_normal(self):
        tests = [0, 10, 12]
        for test in tests:
            src = AudioSrc(port=4000, wave=test)
            assert src.wave == str(test)