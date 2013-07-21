#IMPORTS
#all dbus variables need to be setup here
from gi.repository import Gio, GLib
from exception import *
import sys


CONNECTION_FLAGS = Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT


class Connection(object):
    """Class which makes all remote object class.
    Deals with lower level connection and remote method invoking

    :default bus-address: unix:abstract=gstswitch

    :param: None
    """

    def __init__(
            self,
            address="unix:abstract=gstswitch",
            bus_name=None,
            object_path="/info/duzy/gst/switch/SwitchController",
            default_interface="info.duzy.gst.switch.SwitchControllerInterface"):

        super(Connection, self).__init__()
        self.address = address
        self.bus_name = bus_name
        self.object_path = object_path
        self.default_interface = default_interface

    def connect_dbus(self):
        """Make a new connection using the parameters belonging to the class
        to the gst-switch-srv over dbus.
        Sets the self.connection

        :params: None
        :returns: Nothing
        :raises ConnectionError: GError occurs while making a connection
        """
        try:
            connection = Gio.DBusConnection.new_for_address_sync(
                self.address,
                CONNECTION_FLAGS,
                None,
                None)
            self.connection = connection
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{1} ({0})".format(message, self.address)
            raise ConnectionError(new_message)

    def get_compose_port(self):
        """get_compose_port(out i port);
        Calls get_compose_port remotely

        :param: None
        :returns: tuple with first element compose port number
        """
        try:
            args = None
            connection = self.connection
            port = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'get_compose_port',
                args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return port
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "get_compose_port")
            raise ConnectionError(new_message)

    def get_encode_port(self):
        """get_encode_port(out i port);
        Calls get_encode_port remotely

        :param: None
        :returns: tuple with first element encode port number
        """
        try:
            args = None
            connection = self.connection
            port = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'get_encode_port',
                args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return port
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "get_encode_port")
            raise ConnectionError(new_message)

    def get_audio_port(self):
        """get_audio_port(out i port);
        Calls get_audio_port remotely

        :param: None
        :returns: tuple wit first element audio port number
        """
        try:
            args = None
            connection = self.connection
            port = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'get_audio_port',
                args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return port
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "get_audio_port")
            raise ConnectionError(new_message)

    def get_preview_ports(self):
        """get_preview_ports(out s ports);
        Calls get_preview_ports remotely

        :param: None
        :returns: tuple with first element a string in the form of '[(3002, 1, 7), (3003, 1, 8)]'
        """
        try:
            args = None
            connection = self.connection
            ports = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'get_preview_ports',
                args, GLib.VariantType.new("(s)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return ports
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "get_preview_ports")
            raise ConnectionError(new_message)

    def set_composite_mode(self, mode):
        """set_composite_mode(in  i channel,
                                out b result);
        Calls set_composite_mode remotely

        :param mode: new composite mode
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('(i)', (mode,))
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'set_composite_mode',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "set_composite_mode")
            raise ConnectionError(new_message)

    def set_encode_mode(self, channel):
        """set_encode_mode(in  i channel,
                            out b result);
        Calls set_encode_mode remotely
        **Does not do anything**

        :param: channel
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('(i)', (channel,))
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'set_encode_mode',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "set_encode_mode")
            raise ConnectionError(new_message)

    def new_record(self):
        """new_record(out b result);
        Calls new_record remotely

        :param: None:
        returns: tuple with first element True if requested
        """
        try:
            args = None
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'new_record',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "new_record")
            raise ConnectionError(new_message)

    def adjust_pip(self, dx, dy, dw, dh):
        """adjust_pip(in  i dx,
                           in  i dy,
                           in  i dw,
                           in  i dh,
                           out u result);
        Calls adjust_pip remotely

        :param x: the X position of the PIP
        :param y: the Y position of the PIP
        :param w: the width of the PIP
        :param h: the height of the PIP
        :returns: tuple with first element as result - PIP has been changed succefully
        """
        try:
            args = GLib.Variant('(iiii)', (dx, dy, dw, dh,))
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'adjust_pip',
                args, GLib.VariantType.new("(u)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "adjust_pip")
            raise ConnectionError(new_message)

    def switch(self, channel, port):
        """switch(in  i channel,
                       in  i port,
                       out b result);
        Calls switch remotely

        :param channel: The channel to be switched, 'A', 'B', 'a'
        :param port: The target port number
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('(ii)', (channel, port,))
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'switch',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "switch")
            raise ConnectionError(new_message)

    def click_video(self, x, y, fw, fh):
        """click_video(in  i x,
                            in  i y,
                            in  i fw,
                            in  i fh,
                            out b result);
        Calls click_video remotely

        :param x:
        :param y:
        :param fw:
        :param fh:
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('(iiii)', (x, y, fw, fh,))
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'click_video',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "click_video")
            raise ConnectionError(new_message)

    def mark_face(self, faces):
        """mark_face(in  a(iiii) faces);
        Calls mark_face remotely

        :param faces: tuple having four elements
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('a(iiii)', faces)
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'mark_face',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "mark_face")
            raise ConnectionError(new_message)

    def mark_tracking(self, faces):
        """mark_tracking(in  a(iiii) faces);
        Calls mark_tracking remotely

        :param faces: tuple having four elements
        :returns: tuple with first element True if requested
        """
        try:
            args = GLib.Variant('a(iiii)', faces)
            connection = self.connection
            result = connection.call_sync(
                self.bus_name, self.object_path, self.default_interface, 'mark_tracking',
                args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
                None)
            return result
        except GLib.GError:
            error = sys.exc_info()[1]
            message = error.message
            domain = error.domain
            new_message = "{0}: {1}".format(message, "mark_tracking")
            raise ConnectionError(new_message)
