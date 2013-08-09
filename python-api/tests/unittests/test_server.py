import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
import pytest
from gstswitch.exception import *
import os
import subprocess
from mock import Mock


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


class TestRecordFile(object):
    # Record File
    def test_record_file_blank(self):
        files = ['', None, [], {}]
        path = '/home/hyades/gst/master/gstreamer/tools/'
        for record_file in files:
            with pytest.raises(ValueError):
                Server(path=path, record_file=record_file)

    def test_record_file_slashes(self):
        file = 'abcd/xyz/'
        with pytest.raises(ValueError):
            path = '/home/hyades/gst/master/gstreamer/tools/'
            Server(path=path, record_file=file)


class TestKillTerminate(object):
    # OS Errors
    def test_terminate_fail(self):
        class fake_proc(object):
            def __init__(self):
                pass

            def terminate(self):
                raise OSError

        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.proc = fake_proc()
        with pytest.raises(ServerProcessError):
            s.terminate()

    def test_kill_fail(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        s.proc = 1
        s.pid = -300
        with pytest.raises(ServerProcessError):
            s.kill()

    def test_no_process_kill(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        with pytest.raises(ServerProcessError):
            s.kill()

    def test_no_process_terminate(self):
        path = '/home/hyades/gst/master/gstreamer/tools/'
        s = Server(path=path)
        with pytest.raises(ServerProcessError):
            s.terminate()


class TestRun(object):

    def test_run(self):
        s = Server(path='abc')
        s._run_process = Mock(return_value=MockProcess())
        s.run()
        assert s.pid == 1
        assert s.proc is not None

    def test_run_process(self):
        s = Server(path='abc')
        s._start_process = Mock(return_value=MockProcess())
        s.gst_option_string = ''
        ret = s._run_process()
        assert ret is not None

    def test_start_process_error(self, monkeypatch):
        s = Server(path='abc')
        monkeypatch.setattr(subprocess, 'Popen', Mock(side_effect=OSError))
        with pytest.raises(ServerProcessError):
            s._start_process('cmd')

    def test_start_process_normal(self, monkeypatch):
        s = Server(path='abc')
        monkeypatch.setattr(subprocess, 'Popen', Mock(return_value=MockProcess()))
        s._start_process('cmd') 


class MockProcess(object):
        def __init__(self, mode=True):
            self.mode = mode
            self.pid = 1

        def terminate(self):
            if self.mode == True:
                pass
            if self.mode == False:
                raise OSError('Testing terminate')


class TestNormal(object):
    # Normal Functioning Tests    

    def test_normal_terminate(self):
        s = Server(path='abc')
        s.proc = MockProcess(True)
        s.terminate()
        assert s.proc is None

    def test_normal_kill(self, monkeypatch):
        s = Server(path='abc')
        s.proc = Mock()
        monkeypatch.setattr(os, 'kill', Mock())
        res = s.kill()
        assert  res == True
        assert s.proc is None

    def test_terminate(self):
        s = Server(path='abc')
        s.proc = MockProcess(False)
        with pytest.raises(ServerProcessError):
            s.terminate()

    def test_kill(self, monkeypatch):
        s = Server(path='abc')
        s.proc = Mock()
        monkeypatch.setattr(os, 'kill', Mock(side_effect=OSError))
        with pytest.raises(ServerProcessError):
            s.kill()
