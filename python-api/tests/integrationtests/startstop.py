import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(__file__, "../../../")))

from gstswitch.server import Server
from gstswitch.helpers import *
from gstswitch.controller import Controller
import time
import glob


PATH = '/home/hyades/gst/stage/bin/'

class TestStartStop(object):

    def __init__(self):
        s = Server(PATH)
        try:
            s.run()
            pid = s.pid
            print pid
            if pid < 0 :
                raise OSError('Test Failed')
            else:
                pass
            s.terminate()
        except OSError:
            if s.proc:
                s.kill()
            print "Test Failed"

fails = []
for i in range(5):
    try:
        TestStartStop()
        time.sleep(5)
    except OSError:
        fails.append(i)

print fails

# remove all .data files
for f1 in glob.glob(os.getcwd()+'/*.data'):
    os.remove(f1)