from server import ServerProcess
import pytest


class TestRun(object):

    def test_normal(self):
        instance = ServerProcess()
        instance.VIDEO_PORT = '3000'
        instance.AUDIO_PORT = '4000'
        instance.CONTROL_PORT = '5000'
        instance.RECORD_FILE = 'record.data'
        instance.PATH = '/home/hyades/gst/master/gstreamer/tools/'
        instance.run()
        assert instance.proc.pid >= 0

    def test_no_video_port(self):
        instance = ServerProcess()
        instance.AUDIO_PORT = '4000'
        instance.CONTROL_PORT = '5000'
        instance.RECORD_FILE = 'record.data'
        instance.PATH = '/home/hyades/gst/master/gstreamer/tools/'
        with pytest.raises(AttributeError):
            instance.run()

    def test_no_audio_port(self):
        instance = ServerProcess()
        instance.VIDEO_PORT = '4000'
        instance.CONTROL_PORT = '5000'
        instance.RECORD_FILE = 'record.data'
        instance.PATH = '/home/hyades/gst/master/gstreamer/tools/'
        with pytest.raises(AttributeError):
            instance.run()

    def test_no_control_port(self):
        instance = ServerProcess()
        instance.VIDEO_PORT = '4000'
        instance.AUDIO_PORT = '5000'
        instance.RECORD_FILE = 'record.data'
        instance.PATH = '/home/hyades/gst/master/gstreamer/tools/'
        with pytest.raises(AttributeError):
            instance.run()

