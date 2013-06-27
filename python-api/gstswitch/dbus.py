#IMPORTS


class DBus(object):
    """docstring for DBus"""
    def __init__(self, arg):
        super(DBus, self).__init__()
        self.ADDRESS = "unix:abstract=gstswitch"
        self.BUSNAME = None
        self.OBJECTPATH = "/info/duzy/gst/switch/SwitchController"
        self.DEFAULTINTERFACE = "info.duzy.gst.switch.SwitchControllerInterface"

    def get_address(self):
        return self.ADDRESS

    def get_busname(self):
        return self.BUSNAME

    def get_objectpath(self):
        return self.OBJECTPATH

    def get_default_interface(self):
        return self.DEFAULTINTERFACE
