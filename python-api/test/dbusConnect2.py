from gi.repository import Gio, GLib

def dbus_node_introspect_cb():
	res = connection.call_finish(result_async)
	print res
	node_info = Gio.DBusNodeInfo.new_for_xml(res[0])
	print n
address = "unix:abstract=gstswitch"
name = None
object_path = "/info/duzy/gst_switch/SwitchController"
connection = Gio.DBusConnection.new_for_address_sync(
                    address,
                    Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT,
                    None, None)
print connection
print connection.call(
            name, object_path, 'org.freedesktop.DBus.Introspectable', 'Introspect',
            None, GLib.VariantType.new("(s)"), Gio.DBusCallFlags.NONE, -1,
            None, dbus_node_introspect_cb, object_path)