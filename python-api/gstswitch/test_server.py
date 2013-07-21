from server import Server
import pytest
from exception import *


class TestPath(object):
    # Path Tests
    def test_invalid_path(self):
        path = '/usr/'
        s = Server(path=path)
        with pytest.raises(PathError):
            s.run()

    def test_invalid_path_none(self):
        paths = [None, '', [], {}]
        for path in paths:
            with pytest.raises(ValueError):
                Server(path=path)


class TestVideoPort(object):
    # Video Port Tests
    def test_invalid_video_port_null(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        video_ports = [None, '', [], {}]
        for video_port in video_ports:
            with pytest.raises(ValueError):
                Server(path=path, video_port=video_port)

    def test_invalid_video_port_type(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        video_ports = [[1, 2, 3], {1: 2, 2: 3}]
        for video_port in video_ports:
            with pytest.raises(TypeError):
                Server(path=path, video_port=video_port)

    def test_invalid_video_port_range(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        video_ports = [-99, -1, 1e6]
        for video_port in video_ports:
            with pytest.raises(ValueError):
                Server(path=path, video_port=video_port)


class TestAudioPort(object):
    # Audio Port Tests
    def test_invalid_audio_port_null(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        audio_ports = [None, '', [], {}]
        for audio_port in audio_ports:
            with pytest.raises(ValueError):
                Server(path=path, audio_port=audio_port)

    def test_invalid_audio_port_type(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        audio_ports = [[1, 2, 3], {1: 2, 2: 3}]
        for audio_port in audio_ports:
            with pytest.raises(TypeError):
                Server(path=path, audio_port=audio_port)

    def test_invalid_audio_port_range(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        audio_ports = [-99, -1, 1e6]
        for audio_port in audio_ports:
            with pytest.raises(ValueError):
                Server(path=path, audio_port=audio_port)


class TestControlPort(object):
    # Control Port Tests
    def test_invalid_control_port_null(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        control_ports = [None, '', [], {}]
        for control_port in control_ports:
            with pytest.raises(ValueError):
                Server(path=path, control_port=control_port)

    def test_invalid_control_port_type(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        control_ports = [[1, 2, 3], {1: 2, 2: 3}]
        for control_port in control_ports:
            with pytest.raises(TypeError):
                Server(path=path, control_port=control_port)

    def test_invalid_control_port_range(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        control_ports = [-99, -1, 1e6]
        for control_port in control_ports:
            with pytest.raises(ValueError):
                Server(path=path, control_port=control_port)


class TestNormal(object):
    # Normal Functioning Tests
    def test_normal_terminate(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.run()
        assert s.proc is not None
        s.terminate()
        assert s.proc is None

    def test_normal_kill(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.run()
        assert s.proc is not None
        s.kill()
        assert s.proc is None

    def test_terminate(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        with pytest.raises(ServerProcessError):
            s.terminate()

    def test_kill(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        with pytest.raises(ServerProcessError):
            s.terminate()
