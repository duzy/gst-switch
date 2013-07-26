from controller import Controller
from exception import *
import pytest
from mock import Mock, patch
from connection import Connection


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
        controller = Controller()
        controller.establish_connection()
        assert controller.connection is not None
