from server import BaseServer
import pytest


class TestSetVideoPort(object):

    def test_negative(self):
        port = -3
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_video_port(port)
            assert res is None

    def test_zero(self):
        port = 0
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_video_port(port)
            assert res is None

    def test_huge(self):
        port = 1e6
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_video_port(port)
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_video_port(port)
            assert res is None

    def test_string(self):
        port = ""
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_video_port(port)
            assert res is None

    def test_dict(self):
        port = {}
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_video_port(port)
            assert res is None

    def test_normal(self):
        port = 3000
        instance = BaseServer()
        instance.set_video_port(port)
        assert port == int(instance.VIDEO_PORT)


class TestSetAudioPort(object):

    def test_negative(self):
        port = -3
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_zero(self):
        port = 0
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_huge(self):
        port = 1e6
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_string(self):
        port = ""
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_dict(self):
        port = {}
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_audio_port(port)
            assert res is None

    def test_normal(self):
        port = 3000
        instance = BaseServer()
        instance.set_audio_port(port)
        assert port == int(instance.AUDIO_PORT)


class TestSetControlPort(object):

    def test_negative(self):
        port = -3
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_control_port(port)
            assert res is None

    def test_zero(self):
        port = 0
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_control_port(port)
            assert res is None

    def test_huge(self):
        port = 1e6
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_control_port(port)
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_control_port(port)
            assert res is None

    def test_string(self):
        port = ""
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_control_port(port)
            assert res is None

    def test_dict(self):
        port = {}
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_control_port(port)
            assert res is None

    def test_normal(self):
        port = 3000
        instance = BaseServer()
        instance.set_control_port(port)
        assert port == int(instance.CONTROL_PORT)


class TestSetRecordFile(object):

    def test_int(self):
        name = 1234
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_record_file(name)
            assert res is None

    def test_dict(self):
        name = {1: 5, 2: 7}
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_record_file(name)
            assert res is None

    def test_none(self):
        name = None
        instance = BaseServer()
        with pytest.raises(TypeError):
            res = instance.set_record_file(name)
            assert res is None

    def test_zero_length(self):
        name = ''
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_record_file(name)
            assert res is None

    def test_forward_slash(self):
        name = 'abcd/xyz'
        instance = BaseServer()
        with pytest.raises(ValueError):
            res = instance.set_record_file(name)
            assert res is None

    def test_normal(self):
        name = "record1234"
        instance = BaseServer()
        instance.set_record_file(name)
        assert name == instance.RECORD_FILE


class TestGetVideoPort(object):

    def test_non_string(self):
        port = 1234
        instance = BaseServer()
        instance.VIDEO_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_video_port()
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        instance.VIDEO_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_video_port()
            assert res is None

    def test_zero_length_string(self):
        port = ''
        instance = BaseServer()
        instance.VIDEO_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_video_port()
            assert res is None

    def test_non_int_string(self):
        port = 'abcd'
        instance = BaseServer()
        instance.VIDEO_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_video_port()
            assert res is None

    def test_normal(self):
        port = '3000'
        instance = BaseServer()
        instance.VIDEO_PORT = port
        assert port == instance.get_video_port()


class TestGetAudioPort(object):

    def test_non_string(self):
        port = 1234
        instance = BaseServer()
        instance.AUDIO_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_audio_port()
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        instance.AUDIO_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_audio_port()
            assert res is None

    def test_zero_length_string(self):
        port = ''
        instance = BaseServer()
        instance.AUDIO_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_audio_port()
            assert res is None

    def test_non_int_string(self):
        port = 'abcd'
        instance = BaseServer()
        instance.AUDIO_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_audio_port()
            assert res is None

    def test_normal(self):
        port = '3000'
        instance = BaseServer()
        instance.AUDIO_PORT = port
        assert port == instance.get_audio_port()


class TestGetControlPort(object):

    def test_non_string(self):
        port = 1234
        instance = BaseServer()
        instance.CONTROL_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_control_port()
            assert res is None

    def test_none(self):
        port = None
        instance = BaseServer()
        instance.CONTROL_PORT = port
        with pytest.raises(TypeError):
            res = instance.get_control_port()
            assert res is None

    def test_zero_length_string(self):
        port = ''
        instance = BaseServer()
        instance.CONTROL_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_control_port()
            assert res is None

    def test_non_int_string(self):
        port = 'abcd'
        instance = BaseServer()
        instance.CONTROL_PORT = port
        with pytest.raises(ValueError):
            res = instance.get_control_port()
            assert res is None

    def test_normal(self):
        port = '3000'
        instance = BaseServer()
        instance.CONTROL_PORT = port
        assert port == instance.get_control_port()


class TestGetRecordFile(object):

    def test_non_string(self):
        name = 1234
        instance = BaseServer()
        instance.RECORD_FILE = name
        with pytest.raises(TypeError):
            res = instance.get_record_file()
            assert res is None

    def test_none(self):
        name = None
        instance = BaseServer()
        instance.RECORD_FILE = name
        with pytest.raises(TypeError):
            res = instance.get_record_file()
            assert res is None

    def test_forward_slash(self):
        name = 'abcd/xyz'
        instance = BaseServer()
        instance.RECORD_FILE = name
        with pytest.raises(ValueError):
            res = instance.get_record_file()
            assert res is None

    def test_normal(self):
        name = "recordFile"
        instance = BaseServer()
        instance.RECORD_FILE = name
        assert name == instance.get_record_file()


class TestSetExecutablePath(object):

    def test_integer(self):
        path = 1234
        instance = BaseServer()
        with pytest.raises(TypeError):
            instance.set_executable_path(path)

    def test_none(self):
        path = None
        instance = BaseServer()
        with pytest.raises(TypeError):
            instance.set_executable_path(path)

    def test_zero_length_string(self):
        path = ''
        instance = BaseServer()
        with pytest.raises(ValueError):
            instance.set_executable_path(path)

    def test_normal(self):
        path = "/home/hyades/gst/master/"
        instance = BaseServer()
        instance.set_executable_path(path)
        assert path == instance.PATH
