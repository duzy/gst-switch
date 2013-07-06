#IMPORTS


class DBus(object):
    """docstring for DBus"""
    def __init__(self):
        super(DBus, self).__init__()

    def set_address(self, address):
        self.ADDRESS = address

    def set_busname(self, busname):
        self.BUSNAME = busname

    def set_objectpath(self, objectpath):
        self.OBJECTPATH = objectpath

    def set_default_interface(self, interface):
        self.DEFAULT_INTERFACE = interface

    def get_address(self):
        return self.ADDRESS

    def get_busname(self):
        return self.BUSNAME

    def get_objectpath(self):
        return self.OBJECTPATH

    def get_default_interface(self):
        return self.DEFAULTINTERFACE

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
