from server import Server
import pytest
from exception import *


class TestRun(object):

    def test_invalid_path(self):
        path = '/usr/'
        s = Server(path=path)
        with pytest.raises(PathError):
            s.run()

    def test_invalid_path2(self):
        path = None
        with pytest.raises(ValueError):
            s = Server(path=path)

    def test_invalid_path3(self):
        path = ''
        with pytest.raises(ValueError):
            s = Server(path=path)

    def test_invalid_port(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        video_port = None
        with pytest.raises(ValueError):
            s = Server(path=path, video_port=video_port)

    def test_normal1(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.run()
        assert s.proc is not None
        s.terminate()
        assert s.proc is None

    def test_normal2(self):
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
