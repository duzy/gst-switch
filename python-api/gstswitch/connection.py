#IMPORTS
#all dbus variables need to be setup here
from dbus import DBus
from gi.repository import Gio, GLib


class Connection(DBus):
    """Class which makes all remote object class

    :param: None
    """

    def __init__(self):
        super(Connection, self).__init__()
        self.set_address("unix:abstract=gstswitch")
        self.set_busname(None)
        self.set_objectpath("/info/duzy/gst/switch/SwitchController")
        self.set_default_interface("info.duzy.gst.switch.SwitchControllerInterface")
        # self.connection = self.connect_dbus()

    def get_compose_port(self):
        """get_compose_port(out i port);
        Calls get_compose_port remotely

        :param: None
        :returns: tuple with first element compose port number
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'get_compose_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_encode_port(self):
        """get_encode_port(out i port);
        Calls get_encode_port remotely

        :param: None
        :returns: tuple with first element encode port number
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'get_encode_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_audio_port(self):
        """get_audio_port(out i port);
        Calls get_audio_port remotely

        :param: None
        :returns: tuple wit first element audio port number
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'get_audio_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_preview_ports(self):
        """get_preview_ports(out s ports);
        Calls get_preview_ports remotely

        :param: None
        :returns: tuple with first element a string in the form of '[(3002, 1, 7), (3003, 1, 8)]'
        """
        args = None
        connection = self.get_connection()
        ports = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'get_preview_ports',
            args, GLib.VariantType.new("(s)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return ports

    def set_composite_mode(self, mode):
        """set_composite_mode(in  i channel,
                                out b result);
        Calls set_composite_mode remotely

        :param mode: new composite mode
        :returns: tuple with first element True if requested
        """
        args = GLib.Variant('(i)', (mode,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'set_composite_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def set_encode_mode(self, channel):
        """set_encode_mode(in  i channel,
                            out b result);
        Calls set_encode_mode remotely
        *Does not do anything*

        :param: channel
        :returns: tuple with first element True if requested
        """
        args = GLib.Variant('(i)', (channel,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def new_record(self):
        """new_record(out b result);
        Calls new_record remotely

        :param: None:
        returns: tuple with first element True if requested
        """
        args = None
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'new_record',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

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
        args = GLib.Variant('(iiii)', (dx, dy, dw, dh,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'adjust_pip',
            args, GLib.VariantType.new("(u)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def switch(self, channel, port):
        """switch(in  i channel,
                       in  i port,
                       out b result);
        Calls switch remotely

        :param channel: The channel to be switched, 'A', 'B', 'a'
        :param port: The target port number
        :returns: tuple with first element True if requested
        """
        args = GLib.Variant('(ii)', (channel, port,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'switch',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

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
        args = GLib.Variant('(iiii)', (x, y, fw, fh,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'click_video',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def mark_face(self, faces):
        """mark_face(in  a(iiii) faces);
        Calls mark_face remotely

        :param faces: tuple having four elements
        :returns: tuple with first element True if requested
        """
        args = GLib.Variant('a(iiii)', faces)
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'mark_face',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def mark_tracking(self, faces):
        """mark_tracking(in  a(iiii) faces);
        Calls mark_tracking remotely

        :param faces: tuple having four elements
        :returns: tuple with first element True if requested
        """
        args = GLib.Variant('a(iiii)', faces)
        connection = self.get_connection()
        result = connection.call_sync(
            self.get_busname(), self.get_objectpath(), 'info.duzy.gst.switch.SwitchControllerInterface', 'mark_tracking',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result
