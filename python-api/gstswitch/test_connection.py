import sys
from gi.repository import GLib
from connection import Connection
from exception import *
import pytest


class TestAddress(object):

    def test_address_null(self):
        address = ['', None, [], {}]
        for x in address:
            with pytest.raises(ValueError):
                Connection(address=x)

    def test_address_colon(self):
        address = 'abcdefghijk'
        with pytest.raises(ValueError):
            Connection(address=address)

    def test_address_unknown(self):
        address = [1, 2, 3]
        with pytest.raises(ValueError):
            Connection(address=address)

