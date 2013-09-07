"""Unitests for testsource.py"""
import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.exception import RangeError
from gstswitch.testsource import Preview, VideoSrc
from gstswitch.testsource import BasePipeline, VideoPipeline, AudioSrc
import pytest
from mock import Mock
from gi.repository import Gst


class TestVideoSrcPort(object):
    """Test the port parameter"""
    def test_blank(self):
        """Test when the port is null"""
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=test)

    def test_range(self):
        """Test when the port is not in range"""
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                VideoSrc(port=test)

    def test_invalid(self):
        """Test when the port is not a valid integral value"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=test)

    def test_normal(self):
        """Test when port is a valid value"""
        tests = [1, 65535, 1000]
        for test in tests:
            src = VideoSrc(port=test)
            assert src.port == test

class TestVideoSrcWidth(object):
    """Test the width parameter"""
    def test_blank(self):
        """Test when the width is null"""
        tests = ['', None, [], {}]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, width=test)

    def test_non_positive(self):
        """Test when the width is non-positive"""
        tests = ['-1.111', -1.111, 0, -1e10]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, width=test)

    def test_invalid(self):
        """Test when the width is not a valid float value"""
        tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, width=test)

    def test_normal(self):
        """Test when the width is valid"""
        tests = [  1e6, 300, '200']
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, width=test)
            assert src.width == test


class TestVideoSrcHeight(object):
    """Test for height parameter"""
    def test_blank(self):
        """Test when the height is a null"""
        tests = ['', None, [], {}]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, height=test)

    def test_non_positive(self):
        """Test when heightis non-positive"""
        tests = ['-1.111', -1.111, 0, -1e10]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, height=test)

    def test_invalid(self):
        """Test when height is not a valid float value"""
        tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, height=test)

    def test_normal(self):
        """Test when height is valid"""
        tests = [1e6, 300, '200']
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, height=test)
            assert src.height == test


class TestVideoSrcPattern(object):
    """Test the pattern parameter"""
    def test_range(self):
        """Test when pattern is not in range"""
        tests = [-100, 1e7, 65536, -1, 20]
        port = 1000
        for test in tests:
            with pytest.raises(RangeError):
                VideoSrc(port=port, pattern=test)

    def test_invalid(self):
        """Test when pattern is not a valid integer"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}]
        port = 1000
        for test in tests:
            with pytest.raises(TypeError):
                VideoSrc(port=port, pattern=test)

    def test_normal(self):
        """Test when pattern is valid"""
        tests = [1, 0, 19, 10]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, pattern=test)
            assert src.pattern == str(test)


class TestVideoSrcTimeOverlay(object):
    """Test timeoverlay parameter"""
    def test_fail(self):
        """Test when timeoverlay is not boolean/valid"""
        tests = ['', 1234, 'hi', [1, 2], {1: 2}, None, 0, []]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, timeoverlay=test)

    def test_normal(self):
        """Test when timeoverlay is valid"""
        tests = [True, False]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, timeoverlay=test)
            assert src.timeoverlay == test



class TestVideoSrcClockOverlay(object):
    """Test clockoverlay pattern"""
    def test_fail(self):
        """Test when clockoverlay is not boolean/valid"""
        tests = ['', 1234, 'hi', [1, 2], {1: 2}, None, 0, []]
        port = 1000
        for test in tests:
            with pytest.raises(ValueError):
                VideoSrc(port=port, clockoverlay=test)

    def test_normal(self):
        """Test when clockoverlay is valid"""
        tests = [True, False]
        port = 1000
        for test in tests:
            src = VideoSrc(port=port, clockoverlay=test)
            assert src.clockoverlay == test

class MockPipeline(object):
    """Mock Pipeline"""
    def play(self):
        """Play the pipeline"""
        pass

    def pause(self):
        """Pause the pipeline"""
        pass

    def disable(self):
        """Disable the pipeline"""
        pass


class TestVideoSrcPlay(object):
    """Test Video Source options - play, pause, disable"""
    def test_run(self):
        """Test run method"""
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.run()

    def test_pause(self):
        """Test pause method"""
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.pause()

    def test_end(self):
        """Test end method"""
        src = VideoSrc(port=3000)
        src.pipeline = MockPipeline()
        src.end()


class TestPreviewPort(object):
    """Test port parameter"""
    def test_blank(self):
        """Test when port is null"""
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                Preview(port=test)

    def test_range(self):
        """Test when port is not in range"""
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                Preview(port=test)

    def test_invalid(self):
        """Test when port is not a valid integral value"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                Preview(port=test)

    def test_normal(self):
        """Test when port is valid"""
        tests = [1, 65535, 1000]
        for test in tests:
            src = Preview(port=test)
            assert src.preview_port == test

class TestPreviewPlay(object):
    """Test preview options - play, pause, end"""
    def test_run(self):
        """Test play method"""
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.run()

    def test_pause(self):
        """Test pause method"""
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.pause()

    def test_end(self):
        """Test end method"""
        src = Preview(port=3001)
        src.pipeline = MockPipeline()
        src.end()



class TestBasePipeline(object):
    """Test Base Pipeline"""
    def test_play(self, monkeypatch):
        """Test play method"""
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.play()

    def test_pause(self, monkeypatch):
        """Test pause method"""
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.pause()

    def test_disable(self, monkeypatch):
        """Test disable method"""
        monkeypatch.setattr(Gst.Pipeline, '__init__', Mock())
        pipeline = BasePipeline()
        pipeline.set_state = Mock()
        pipeline.disable()


class TestVideoPipeline(object):
    """Test VideoPipeline"""
    def test_permuate_time_clock_1(self):
        """Test when timeoverlay False and clockoverlay False"""
        VideoPipeline(
            port=3000,
            pattern=10,
            timeoverlay=False,
            clockoverlay=False)

    def test_permuate_time_clock_2(self):
        """test when timeoverlay False and clockoverlay True"""
        VideoPipeline(
            port=3000,
            pattern=10,
            timeoverlay=False,
            clockoverlay=True)

    def test_permuate_time_clock_3(self):
        """Test when timeoverlay True and clockoverlay False"""
        VideoPipeline(
            port=3000,
            pattern=10,
            timeoverlay=True,
            clockoverlay=False)

    def test_permuate_time_clock_4(self):
        """Test when timeoverlay True and clockoverlay True"""
        VideoPipeline(
            port=3000,
            pattern=10,
            timeoverlay=True,
            clockoverlay=True)


class TestAudioSrcPort(object):
    """Test port parameter"""
    def test_blank(self):
        """Test when port is null"""
        tests = ['', None, [], {}]
        for test in tests:
            with pytest.raises(ValueError):
                AudioSrc(port=test)

    def test_range(self):
        """Test when port is not in range"""
        tests = [-100, 1e7, 65536]
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=test)

    def test_invalid(self):
        """Test when port is not a valid integral value"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=test)

    def test_normal(self):
        """Test when port is valid"""
        tests = [1, 65535, 1000]
        for test in tests:
            src = AudioSrc(port=test)
            assert src.port == test


class TestAudioSrcFreq(object):
    """Test frequency parameter"""
    def test_blank(self):
        """Test when frequency is null"""
        tests = ['', None, [], {}, 0]
        port = 4000
        for test in tests:
            with pytest.raises(ValueError):
                AudioSrc(port=port, freq=test)

    def test_range(self):
        """Test when frequency is not in range (negative)"""
        tests = [-100,  -1]
        port = 4000
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=port, freq=test)

    def test_invalid(self):
        """Test when frequency is not a valid integral value"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
        port = 4000
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=port, freq=test)

    def test_normal(self):
        """Test when frequency is valid"""
        tests = [1, 65535, 1000]
        for test in tests:
            src = AudioSrc(port=4000, freq=test)
            assert src.freq == test


class TestAudioSrcWave(object):
    """Test wave parameter"""
    def test_range(self):
        """Test when wave is not in range"""
        tests = [-100,  -1, 13, 1e2, '1e10']
        port = 4000
        for test in tests:
            with pytest.raises(RangeError):
                AudioSrc(port=port, wave=test)

    def test_invalid(self):
        """Test when wave is not a valid integral value"""
        tests = [[1, 2, 3, 4], {1: 2, 2: 3},]
        port = 4000
        for test in tests:
            with pytest.raises(TypeError):
                AudioSrc(port=port, wave=test)

    def test_normal(self):
        """Test when wave is valid"""
        tests = [0, 10, 12]
        for test in tests:
            src = AudioSrc(port=4000, wave=test)
            assert src.wave == str(test)


class TestAudioSrcPlay(object):
    """Test Audio Source options - play, pause, end"""
    def test_run(self):
        """Run the audio source"""
        src = AudioSrc(port=3000)
        src.pipeline = MockPipeline()
        src.run()

    def test_pause(self):
        """Pause the audio source"""
        src = AudioSrc(port=3000)
        src.pipeline = MockPipeline()
        src.pause()

    def test_end(self):
        """End the audio source"""
        src = AudioSrc(port=3000)
        src.pipeline = MockPipeline()
        src.end()
