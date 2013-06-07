#!/usr/bin/python
from gi.repository import Gio


DBUS_ADDR = 'unix:abstract=gstswitch'

flags = Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT

conn = Gio.DBusConnection.new_for_address_sync(DBUS_ADDR, flags, None, None)
print "BUS:",conn

name = 'info.duzy.gst_switch.SwitchClientInterface'
obj_path = '/info/duzy/gst_switch/SwitchController'
iname = '/info/duzy/gst_switch/SwitchController'
proxy_flags = Gio.DBusProxyFlags.NONE
proxy = Gio.DBusProxy.new_sync(conn, proxy_flags, None, name, obj_path, iname, None)


print proxy