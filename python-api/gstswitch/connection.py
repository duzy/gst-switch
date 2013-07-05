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

    def connect_dbus(self):
        CONNECTION_FLAGS = Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT
        connection = Gio.DBusConnection.new_for_address_sync(
            self.address,
            CONNECTION_FLAGS,
            None,
            None)
        self.connection = connection

    def get_connection(self):
        return self.connection
