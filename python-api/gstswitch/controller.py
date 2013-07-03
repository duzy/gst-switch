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

    def set_composite_mode(self, mode):
        if mode >= 0 and mode <= 3:
            res = self.connection.set_composite_mode(mode)
            if res is True:
                logging.info()
        else:
            pass
            #raise Exception

    def get_compose_port(self):
        pass
