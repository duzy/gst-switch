from controller import Controller
from exception import ConnectionReturnError
import pytest
from mock import Mock, patch
from connection import Connection
from gi.repository import GLib


class TestAddress(object):

    def test_address_null(self):
        address = ['', None, [], {}]
        for x in address:
            with pytest.raises(ValueError):
                Controller(address=x)

    def test_address_colon(self):
        address = 'abcdefghijk'
        with pytest.raises(ValueError):
            Controller(address=address)

    def test_address_normal(self):
        address = ['unix:abstract=gstswitch', 'unix:temp=/tmp/abcd/xyz']
        for x in address:
            conn = Controller(address=x)
            assert conn.address == x


class TestBusName(object):

    def test_normal(self):
        names = ['', 'abcd', 12345]
        for bus in names:
            conn = Controller(bus_name=bus)
            assert conn.bus_name == str(bus)

    def test_normal_none(self):
        name = None
        conn = Controller(bus_name=name)
        assert conn.bus_name == name


class TestObjectPath(object):

    def test_object_path_blank(self):
        paths = [None, '', {}, []]
        for object_path in paths:
            with pytest.raises(ValueError):
                Controller(object_path=object_path)

    def test_object_path_slash(self):
        object_path = 'a/////////'
        with pytest.raises(ValueError):
            Controller(object_path=object_path)

    def test_object_path_normal(self):
        object_path = "/info/duzy/gst/switch/SwitchController"
        conn = Controller(object_path=object_path)
        assert conn.object_path == object_path


class TestInterface(object):

    def test_interface_none(self):
        default_interface = [None, '', [], {}]
        for x in default_interface:
            with pytest.raises(ValueError):
                Controller(default_interface=x)

    def test_interface_dot(self):
        default_interface = ['.', 'info.', 'info']
        for x in default_interface:
            with pytest.raises(ValueError):
                Controller(default_interface=x)

    def test_interface_normal(self):
        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Controller(default_interface=default_interface)
        assert default_interface == conn.default_interface


class TestEstablishConnection(object):

    def test_normal(self, monkeypatch):
        monkeypatch.setattr(Connection, 'connect_dbus', Mock())
        controller = Controller(address='unix:abstract=abcd')
        controller.establish_connection()
        assert controller.connection is not None


class MockConnection(object):

        def __init__(self, mode):
            self.mode = mode

        def establish_connection(self):
            pass

        def get_compose_port(self):
            if self.mode is False:
                return GLib.Variant('(i)', (3001,))
            else:
                return (0,)

        def get_encode_port(self):
            if self.mode is False:
                return GLib.Variant('(i)', (3002,))
            else:
                return (0,)

        def get_audio_port(self):
            if self.mode is False:
                return GLib.Variant('(i)', (4000,))
            else:
                return (0,)

        def get_preview_ports(self):
            if self.mode is False:
                return GLib.Variant('(s)', ('[(3002, 1, 7), (3003, 1, 8)]',))
            else:
                return (0,)

        def set_composite_mode(self, m):
            if self.mode is False:
                return GLib.Variant('(b)', (True,))
            else:
                return (False,)

        def set_encode_mode(self, m):
            if self.mode is False:
                return GLib.Variant('(b)', (True,))
            else:
                return (True,)

        def new_record(self):
            if self.mode is False:
                return GLib.Variant('(b)', (True,))
            else:
                return (True,)

        def adjust_pip(self, xpos, ypos, width, height):
            if self.mode is False:
                return GLib.Variant('(u)', (1,))
            else:
                return (1,)

        def switch(self, channel, port):
            if self.mode is False:
                return GLib.Variant('(b)', (True,))
            else:
                return (True,)

        def click_video(self, xpos, ypos, width, height):
            if self.mode is False:
                return GLib.Variant('(b)', (True,))
            else:
                return (True,)

        def mark_face(self, face):
            pass

        def mark_tracking(self, face):
            pass
            


class TestGetComposePort(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcdefghijk')
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.get_compose_port()

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.connection = MockConnection(False)
        assert controller.get_compose_port() == 3001


class TestGetEncodePort(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcdefghijk')
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.get_encode_port()

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.connection = MockConnection(False)
        assert controller.get_encode_port() == 3002


class TestGetAudioPort(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcdefghijk')
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.get_audio_port()

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.connection = MockConnection(False)
        assert controller.get_audio_port() == 4000


class TestGetPreviewPorts(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcdefghijk')
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.get_preview_ports()

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.connection = MockConnection(False)
        controller._parse_preview_ports = Mock(return_value=[3001, 3002])
        assert controller.get_preview_ports() == [3001, 3002]


class TestSetCompositeMode(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcdefghijk')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.set_composite_mode(1)

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.set_composite_mode(1) == True



class TestSetEncodeMode(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.set_encode_mode(1)

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.set_encode_mode(1) == True


class TestNewRecord(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.new_record()

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.new_record() == True


class TestAdjustPIP(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.adjust_pip(1,2,3,4)

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.adjust_pip(1,2,3,4) == 1



class TestSwitch(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.switch(1,2)

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.switch(1,2) == True



class TestClickVideo(object):

    def test_unpack(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        with pytest.raises(ConnectionReturnError):
            controller.click_video(1,2,3,4)

    def test_normal_unpack(self):
        controller =  Controller(address='unix:abstract=abcdef')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(False)
        assert controller.click_video(1,2,3,4) == True


class TestMarkFaces(object):

    def test_normal(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        face = [(1, 2, 3, 4), (1, 1, 1, 1)]
        controller.mark_face(face)


class TestMarkTracking(object):

    def test_normal(self):
        controller = Controller(address='unix:abstract=abcde')
        controller.establish_connection = Mock(return_value=None)
        controller.connection = MockConnection(True)
        face = [(1, 2, 3, 4), (1, 1, 1, 1)]
        controller.mark_tracking(face)

class TestParsePreviewPorts(object):

    def test_value_error(self):
        controller = Controller(address='unix:abstract=abcde')
        test = 1234
        with pytest.raises(ConnectionReturnError):
            controller._parse_preview_ports(test)

    def test_syntax_error(self):
        controller = Controller(address='unix:abstract=abcde')
        test = '{}[]'
        with pytest.raises(ConnectionReturnError):
            controller._parse_preview_ports(test)

    def test_normal(self):
        controller = Controller(address='unix:abstract=abcde')
        test = '[(1, 2, 3), (2, 2, 2)]'
        assert controller._parse_preview_ports(test) == [1, 2]
