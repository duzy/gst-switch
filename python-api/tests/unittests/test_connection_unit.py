import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.connection import Connection
from gstswitch.exception import *
import pytest
from gi.repository import Gio, GLib
from mock import Mock


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

    def test_mock1(self, monkeypatch):

        monkeypatch.setattr(Gio.DBusConnection, 'new_for_address_sync', Mock(side_effect=GLib.GError))
        conn = Connection()
        with pytest.raises(ConnectionError):
            conn.connect_dbus()

    def test_mock2(self, monkeypatch):

        monkeypatch.setattr(Gio.DBusConnection, 'new_for_address_sync', Mock(return_value=1))
        conn = Connection()
        conn.connect_dbus()
        assert conn.connection is not None


class MockConnection(object):

    funs = {
        'get_compose_port': (3001,),
        'get_encode_port': (3002,),
        'get_audio_port': (4000,),
        'get_preview_ports': ('[(3002, 1, 7), (3003, 1, 8)]',),
        'set_composite_mode': (False,),
        'set_encode_mode': (False,),
        'new_record': (False,),
        'adjust_pip': (1,),
        'switch': (True,),
        'click_video': (True,),
        'mark_face': None,
        'mark_tracking': None
    }

    def __init__(self, method):
        self.method = method
        self.return_result = self.funs[method]

    def call_sync(
            self,
            bus_name,
            object_path,
            interface_name,
            method_name,
            parameters,
            reply_type, flags,
            timeout_msec,
            cancellable):
        if interface_name == "info.duzy.gst.switch.SwitchControllerInterface":
            return self.return_result
        else:
            raise GLib.GError('{0}: Test Failed'.format(self.method))


def test_get_compose_port():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_compose_port')
        with pytest.raises(ConnectionError):
            conn.get_compose_port()

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_compose_port')
        assert conn.get_compose_port() == (3001,)


def test_get_encode_port():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_encode_port')
        with pytest.raises(ConnectionError):
            conn.get_encode_port()

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_encode_port')
        assert conn.get_encode_port() == (3002,)


def test_get_audio_port():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_audio_port')
        with pytest.raises(ConnectionError):
            conn.get_audio_port()

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_audio_port')
        assert conn.get_audio_port() == (4000,)


def test_get_preview_ports():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_preview_ports')
        with pytest.raises(ConnectionError):
            conn.get_preview_ports()

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('get_preview_ports')
        assert conn.get_preview_ports() == ('[(3002, 1, 7), (3003, 1, 8)]',)


def test_set_composite_mode():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('set_composite_mode')
        with pytest.raises(ConnectionError):
            conn.set_composite_mode(2)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('set_composite_mode')
        assert conn.set_composite_mode(2) == (False,)


def test_set_encode_mode():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('set_encode_mode')
        with pytest.raises(ConnectionError):
            conn.set_encode_mode(2)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('set_encode_mode')
        assert conn.set_encode_mode(2) == (False,)


def test_new_record():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('new_record')
        with pytest.raises(ConnectionError):
            conn.new_record()

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('new_record')
        assert conn.new_record() == (False,)


def test_adjust_pip():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('adjust_pip')
        with pytest.raises(ConnectionError):
            conn.adjust_pip(1, 2, 3, 4)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('adjust_pip')
        assert conn.adjust_pip(1, 2, 3, 4) == (1,)


def test_switch():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('switch')
        with pytest.raises(ConnectionError):
            conn.switch(1, 2)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('switch')
        assert conn.switch(1, 2) == (True,)


def test_click_video():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('click_video')
        with pytest.raises(ConnectionError):
            conn.click_video(1, 2, 3, 4)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('click_video')
        assert conn.click_video(1, 2, 3, 4) == (True,)


def test_mark_face():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('mark_face')
        with pytest.raises(ConnectionError):
            face = [(1, 1, 1, 1), (2, 2, 2, 2)]
            conn.mark_face(face)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('mark_face')
        face = [(1, 1, 1, 1), (2, 2, 2, 2)]
        assert conn.mark_face(face) is None


def test_mark_tracking():

        default_interface = "info.duzy.gst.switch"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('mark_tracking')
        with pytest.raises(ConnectionError):
            face = [(1, 1, 1, 1), (2, 2, 2, 2)]
            conn.mark_tracking(face)

        default_interface = "info.duzy.gst.switch.SwitchControllerInterface"
        conn = Connection(default_interface=default_interface)
        conn.connection = MockConnection('mark_tracking')
        face = [(1, 1, 1, 1), (2, 2, 2, 2)]
        assert conn.mark_tracking(face) is None
