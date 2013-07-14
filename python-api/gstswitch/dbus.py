#IMPORTS
from gi.repository import Gio


class DBus(object):
    """Class which connects to the dbus
    """
    def __init__(self):
        super(DBus, self).__init__()

    def set_address(self, address):
        """Sets the bus address

        :param address: New bus address
        :returns: nothing
        """
        self.ADDRESS = address

    def set_busname(self, busname):
        """Sets the bus name
        Not used for default usage with gst-switch

        :param busname: New bus name
        :returns: nothing
        """
        self.BUSNAME = busname

    def set_objectpath(self, objectpath):
        """Sets the object path

        :param objectpath: The new object paths
        :returns: Nothing
        """
        self.OBJECTPATH = objectpath

    def set_default_interface(self, interface):
        """Sets the object's default interface

        :param interface: The new interfaces
        :returns: nothing
        """
        self.DEFAULT_INTERFACE = interface

    def get_address(self):
        """Returns the bus address

        :params: None
        :returns: Bus Address
        """
        return self.ADDRESS

    def get_busname(self):
        """Returns the bus name

        :params: None
        :returns: Bus Name
        """
        return self.BUSNAME

    def get_objectpath(self):
        """Returns the object path

        :params: None
        :returns: Object path
        """
        return self.OBJECTPATH

    def get_default_interface(self):
        """Returns the object's default interface

        :params: None
        :returns: Default interface
        """
        return self.DEFAULTINTERFACE

    def connect_dbus(self):
        """Make a new connection using the parameters belonging to the class
        to the gst-switch-srv over dbus.
        Sets the self.connection

        :params: None
        :returns: Nothing
        """
        CONNECTION_FLAGS = Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT
        connection = Gio.DBusConnection.new_for_address_sync(
            self.ADDRESS,
            CONNECTION_FLAGS,
            None,
            None)
        self.connection = connection

    def get_connection(self):
        """Returns the connection made over dbus

        :paramss: None
        :returns: connection
        """
        return self.connection
