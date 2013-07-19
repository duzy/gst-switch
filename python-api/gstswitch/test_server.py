from server import Server
import pytest
from exception import *


class TestRun(object):

    def test_invalid_path(self):
        path = '/usr/'
        s = Server(path=path)
        with pytest.raises(PathError):
            s.run()

    def test_normal(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.run()
        assert s.proc is not None
        s.terminate()

    def test_terminate(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        with pytest.raises(ServerProcessError):
            s.terminate()

    # def test_kill(self):
    #     pass
