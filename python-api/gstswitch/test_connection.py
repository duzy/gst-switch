from connection import Connection
from exception import *
import pytest


class TestAddress(object):

    def test_address_null(self):
        address = ['', None, [], {}]
        for x in address:
            with pytest.raises(ValueError):
                Connection(address=x)

    def test_address_colon(self):
        address = 'abcdefghijk'
        with pytest.raises(ValueError):
            Connection(address=address)

    def test_address_normal(self):
        address = ['unix:abstract=gstswitch', 'unix:temp=/tmp/abcd/xyz']
        for x in address:
            conn = Connection(address=x)
            assert conn.address == x


class TestBusName(object):

    def test_normal(self):
        names = ['', 'abcd', 12345]
        for bus in names:
            conn = Connection(bus_name=bus)
            assert conn.bus_name == str(bus)

    def test_normal_none(self):
        name = None
        conn = Connection(bus_name=name)
        assert conn.bus_name == name


class TestObjectPath(object):

    def test_object_path_blank(self):
        paths = [None, '', {}, []]
        for object_path in paths:
            with pytest.raises(ValueError):
                Connection(object_path=object_path)

    def test_object_path_slash(self):
        object_path = 'a/////////'
        with pytest.raises(ValueError):
            Connection(object_path=object_path)

    def test_object_path_normal(self):
        object_path = "/info/duzy/gst/switch/SwitchController"
        conn = Connection(object_path=object_path)
        assert conn.object_path == object_path


class TestInterface(object):

    def test_interface_none(self):
        default_interface = [None, '', [], {}]
        for x in default_interface:
            with pytest.raises(ValueError):
                Connection(default_interface=x)

    def test_interface_dot(self):
        default_interface = ['.', 'info.', 'info']
        for x in default_interface:
            with pytest.raises(ValueError):
                Connection(default_interface=x)

    def test_interface_normal(self):
        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        assert default_interface == conn.default_interface


class TestConnectDBus(object):

    def test_bad_address(self):
        address = 'unix:path=gstswitch'
        conn = Connection(address=address)
        with pytest.raises(ConnectionError):
            conn.connect_dbus()

    def test_bad_address2(self):
        address = 'unix:temp=gstswitch'
        conn = Connection(address=address)
        with pytest.raises(ConnectionError):
            conn.connect_dbus()

    def test_bad_address3(self):
        address = 'unix:path'
        conn = Connection(address=address)
        with pytest.raises(ConnectionError):
            conn.connect_dbus()

    def test_normal(self):
        s = Server()
        conn = Connection(address=address)