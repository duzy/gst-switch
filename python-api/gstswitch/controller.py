#IMPORTS
from gi.repository import Gio, GLib
import logging

from connection import Connection

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class Controller(object):
    """docstring for Controller"""
    def __init__(self):
        super(Controller, self).__init__()

    def establish_connection(self):
        self.connection = Connection()
        self.connection.connect_dbus

    def get_compose_port(self):
        conn = self.connection.get_compose_port()
        compose_port = conn.unpack()[0]
        return compose_port

    def get_encode_port(self):
        conn = self.connection.get_encode_port()
        encode_port = conn.unpack()[0]
        return encode_port

    def get_audio_port(self):
        conn = self.connection.get_audio_port()
        audio_port = conn.unpack()[0]
        return audio_port

    def get_preview_ports(self):
        conn = self.connection.get_preview_ports()
        preview_ports = conn.unpack[0]
        # TO-DO: implement a parser
        return preview_ports

    def set_composite_mode(self, mode):
        # only modes from 0 to 3 are supported
        if mode >= 0 and mode <= 3:
            conn = self.connection.set_composite_mode(mode)
            res = conn.unpack()[0]
            if res is True:
                logging.info("Set composite mode to %s" % (str(mode)))
        else:
            pass
            # raise some Exception
        return res

    def set_encode_mode(self, channel):
        conn = self.connection.set_encode_mode(channel)
        res = conn.unpack()[0]
        if res is True:
            logging.info("Set encode mode to %s" % (str(channel)))
        else:
            pass
            # raise some exception
        return res

    def new_record(self):
        conn = self.connection.new_record()
        res = conn.unpack()[0]
        if res is True:
            #logging
            logging.info("New record")
        else:
            pass
        return res

    def adjust_pip(self, dx, dy, dw, dh):
        conn = self.connection.adjust_pip(dx, dy, dw, dh)
        res = conn.unpack()[0]
        logging.info("adjust pip dx:%s dy:%s dw:%s dh:%s" % (str(dx), str(dy), str(dw), str(dh)))
        #to-do - parse
        return res

    def switch(self, channel, port):
        conn = self.connection.switch(channel, port)
        res = conn.unpack()[0]
        if res is True:
            logging.info("Switch channel:%s port:%s" % (str(channel), str(port)))
        else:
            pass

    def click_video(self, x, y, fw, fh):
        conn = self.connection.click_video(x, y, fw, fh)
        res = conn.unpack()[0]
        if res is True:
            logging.info("Click video: x:%s y:%s fw:%s fh:%s" % (str(x), str(y), str(fw), str(fh)))
        else:
            pass
        return res

    def mark_face(self, faces):
        # faces is dictionary
        self.connection.mark_face(faces)

    def mark_tracking(self, faces):
        self.connection.mark_tracking(faces)
