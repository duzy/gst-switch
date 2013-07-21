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

    def test_address_normal(self):
        address = ['unix:abstract=gstswitch', 'unix:temp=/tmp/abcd/xyz']
        for x in address:
            conn = Connection(address=x)
            assert conn.address == x


class TestBusName(object):

    def test_normal(self):
        names = ['', 'abcd', 12345]
        for bus in names:
            conn = Connection(bus_name=bus)
            assert conn.bus_name == str(bus)

    def test_normal_none(self):
        name = None
        conn = Connection(bus_name=name)
        assert conn.bus_name == name
