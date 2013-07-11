#IMPORTS
#all dbus variables need to be setup here
from dbus import DBus
from gi.repository import Gio, GLib


class Connection(DBus):
    """docstring for Connection"""

    def __init__(self):
        super(Connection, self).__init__()
        self.set_address("unix:abstract=gstswitch")
        self.set_busname(None)
        self.set_objectpath("/info/duzy/gst/switch/SwitchController")
        self.set_default_interface("info.duzy.gst.switch.SwitchControllerInterface")
        # self.connection = self.connect_dbus()

    def get_compose_port(self):
        """get_compose_port(out i port);
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'get_compose_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_encode_port(self):
        """get_encode_port(out i port);
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'get_encode_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_audio_port(self):
        """get_audio_port(out i port);
        """
        args = None
        connection = self.get_connection()
        port = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'get_audio_port',
            args, GLib.VariantType.new("(i)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return port

    def get_preview_ports(self):
        """get_preview_ports(out s ports);
        """
        args = None
        connection = self.get_connection()
        ports = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'get_preview_ports',
            args, GLib.VariantType.new("(s)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return ports

    def set_composite_mode(self, mode):
        """set_composite_mode(in  i channel,
                                out b result);
        """
        args = GLib.Variant('(i)', (mode,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_composite_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def set_encode_mode(self, channel):
        """set_encode_mode(in  i channel,
                            out b result);
        """
        args = GLib.Variant('(i)', (channel,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def new_record(self):
        """new_record(out b result);
        """
        args = None
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'new_record',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def adjust_pip(self, dx, dy, dw, dh):
        """adjust_pip(in  i dx,
                           in  i dy,
                           in  i dw,
                           in  i dh,
                           out u result);
        """
        args = GLib.Variant('(iiii)', (dx, dy, dw, dh,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'adjust_pip',
            args, GLib.VariantType.new("(u)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def switch(self, channel, port):
        """switch(in  i channel,
                       in  i port,
                       out b result);
        """
        args = GLib.Variant('(ii)', (channel, port,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def click_video(self, x, y, fw, fh):
        """click_video(in  i x,
                            in  i y,
                            in  i fw,
                            in  i fh,
                            out b result);
        """
        args = GLib.Variant('(iiii)', (x, y, fw, fh,))
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def mark_face(self, faces):
        """mark_face(in  a(iiii) faces);
        """
        args = GLib.Variant('a(iiii)', faces)
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result

    def mark_tracking(self, faces):
        """mark_tracking(in  a(iiii) faces);
        """
        args = GLib.Variant('a(iiii)', faces)
        connection = self.get_connection()
        result = connection.call_sync(
            self.name, self.object_path, 'info.duzy.gst.switch.SwitchControllerInterface', 'set_encode_mode',
            args, GLib.VariantType.new("(b)"), Gio.DBusCallFlags.NONE, -1,
            None)
        return result
