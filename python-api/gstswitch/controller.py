#IMPORTS
import ast
from connection import Connection
from exception import *

__all__ = ["Controller", ]


class Controller(object):
    """A Class to control all interactions with the gst-switch-srv over dbus.
    Provides the interface for higher level interactions

    :param: None
    """
    def __init__(
            self,
            address="unix:abstract=gstswitch",
            bus_name=None,
            object_path="/info/duzy/gst/switch/SwitchController",
            default_interface="info.duzy.gst.switch.SwitchControllerInterface"):

        super(Controller, self).__init__()
        self.address = address
        self.bus_name = bus_name
        self.object_path = object_path
        self.default_interface = default_interface

    @property
    def address(self):
        return self._address

    @address.setter
    def address(self, address):
        """Set the Address
        http://dbus.freedesktop.org/doc/dbus-specification.html#addresses
        """
        if not address:
            raise ValueError("Address '{0}' cannot be blank")
        else:
            a = str(address)
            if a.find(':') > 0:
                self._address = a
            else:
                raise ValueError("Address must follow specifications mentioned at "
                                 "http://dbus.freedesktop.org/doc/dbus-specification.html#addresses")

    @property
    def bus_name(self):
        if self._bus_name is None:
            return None
        return self._bus_name

    @bus_name.setter
    def bus_name(self, bus_name):
        """Set the Bus Name
        http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-names-bus
        """
        if bus_name is None:
            self._bus_name = None
            return
        a = str(bus_name)
        self._bus_name = a

    @property
    def object_path(self):
        return self._object_path

    @object_path.setter
    def object_path(self, object_path):
        """Set the object_path
        http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-marshaling-object-path
        """
        if not object_path:
            raise ValueError("object_path '{0} cannot be blank'")
        else:
            a = str(object_path)
            if a[0] == '/':
                self._object_path = a
            else:
                raise ValueError("object_path must follow specifications mentioned at "
                                 "http://dbus.freedesktop.org/doc/dbus-specification.html"
                                 "#message-protocol-marshaling-object-path")

    @property
    def default_interface(self):
        return self._default_interface

    @default_interface.setter
    def default_interface(self, default_interface):
        """Set the default_interface
        http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-names-interface
        """
        if not default_interface:
            raise ValueError("default_interface '{0} cannot be blank'")
        else:
            a = str(default_interface)
            if a.count('.') > 1:
                self._default_interface = a
            else:
                raise ValueError("default_interface must follow specifications mentioned at "
                                 "http://dbus.freedesktop.org/doc/dbus-specification.html"
                                 "#message-protocol-names-interface")

    def establish_connection(self):
        """Establishes a fresh connection to the dbus
        Connection stored as self.connection

        :param: None
        :returns: None
        """
        try:
            self.connection = Connection(
                address=self.address,
                bus_name=self.bus_name,
                object_path=self.object_path,
                default_interface=self.default_interface)

            self.connection.connect_dbus()
        except (ConnectionError, ValueError) as e:
            raise

    def get_compose_port(self):
        """Get the compose port number

        :param: None
        :returns: compose port number
        """
        conn = self.connection.get_compose_port()
        compose_port = conn.unpack()[0]
        return compose_port

    def get_encode_port(self):
        """Get the encode port number

        :param: None
        :returns: encode port number
        """
        conn = self.connection.get_encode_port()
        encode_port = conn.unpack()[0]
        return encode_port

    def get_audio_port(self):
        """Get the audio port number

        :param: None
        :returns: audio port number
        """
        conn = self.connection.get_audio_port()
        audio_port = conn.unpack()[0]
        return audio_port

    def get_preview_ports(self):
        """Get all the preview ports

        :param: None
        :returns: list of all preview ports
        """
        conn = self.connection.get_preview_ports()
        res = conn.unpack()[0]
        preview_ports = self._parse_preview_ports(res)
        return preview_ports

    def set_composite_mode(self, mode):
        """Set the current composite mode. Modes between 0 and 3 are allowed.

        :param mode: new composite mode
        :returns: True when requested
        """
        self.establish_connection()
        # only modes from 0 to 3 are supported
        if mode >= 0 and mode <= 3:
            conn = self.connection.set_composite_mode(mode)
            res = conn.unpack()[0]
            if res is True:
                print "Set composite mode to %s" % (str(mode))
        else:
            pass
            # raise some Exception
        return res

    def set_encode_mode(self, channel):
        """Set the encode mode
        WARNING: THIS DOES NOT WORK.

        :param: channel
        :returns: True when requested
        """
        self.establish_connection()
        conn = self.connection.set_encode_mode(channel)
        res = conn.unpack()[0]
        if res is True:
            print "Set encode mode to %s" % (str(channel))
        else:
            pass
            # raise some exception
        return res

    def new_record(self):
        """Start a new recording

        :param: None
        """
        self.establish_connection()
        conn = self.connection.new_record()
        res = conn.unpack()[0]
        if res is True:
            #logging
            print "New record"
        else:
            pass
        return res

    def adjust_pip(self, x, y, w, h):
        """Change the PIP position and size

        :param x: the X position of the PIP
        :param y: the Y position of the PIP
        :param w: the width of the PIP
        :param h: the height of the PIP
        :returns: result - PIP has been changed succefully
        """
        self.establish_connection()
        conn = self.connection.adjust_pip(x, y, w, h)
        res = conn.unpack()[0]
        print "adjust pip x:%s y:%s w:%s h:%s" % (str(x), str(y), str(w), str(h))
        #to-do - parse
        return res

    def switch(self, channel, port):
        """Switch the channel to the target port

        :param channel: The channel to be switched, 'A', 'B', 'a'
        :param port: The target port number
        :returns: True when requested
        """
        self.establish_connection()
        conn = self.connection.switch(channel, port)
        res = conn.unpack()[0]
        if res is True:
            print "Switch channel:%s port:%s" % (str(channel), str(port))
        else:
            pass

    def click_video(self, x, y, fw, fh):
        """User click on the video

        :param x:
        :param y:
        :param fw:
        :param fh:
        :returns: True when requested
        """
        self.establish_connection()
        conn = self.connection.click_video(x, y, fw, fh)
        res = conn.unpack()[0]
        if res is True:
            print "Click video: x:%s y:%s fw:%s fh:%s" % (str(x), str(y), str(fw), str(fh))
        else:
            pass
        return res

    def mark_face(self, faces):
        """Mark faces

        :param faces: tuple having four elements
        :returns: True when requested
        """
        # faces is dictionary
        self.establish_connection()
        self.connection.mark_face(faces)

    def mark_tracking(self, faces):
        """Mark tracking

        :param faces: tuple having four elements
        :returns: True when requested
        """
        self.establish_connection()
        self.connection.mark_tracking(faces)

    def _parse_preview_ports(self, res):
        # res = '[(a, b, c), (a, b, c)*]'
        x = ast.literal_eval(res)
        preview_ports = []
        for tupl in x:
            preview_ports.append(int(tupl[0]))
        return preview_ports
