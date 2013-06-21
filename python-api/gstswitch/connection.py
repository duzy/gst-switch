#IMPORTS
from dbus import DBus
from gi.repository import Gst, GLib



class Connection(DBus):
	"""docstring for Connection"""


	def __init__(self):
		super(Connection, self).__init__()
		self.connection = self.connect_dbus()

	def connect_dbus(self):
		CONNECTION_FLAGS = Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT
		connection = Gio.DBusConnection.new_for_address_sync(
                    self.address,
                    CONNECTION_FLAGS,
                    None, None)
		return connection

	def get_connection(self):
		return self.connection