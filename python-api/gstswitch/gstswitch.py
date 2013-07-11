# from controller import Controller
# from testsource import VideoSrc
from server import Server
import os
import sys
import signal
import subprocess
import random

#IMPORTS


class UI(object):
    """docstring for UI"""

    def __init__(self):
        """Constructor for the UI Object.
        Returns:
            UI() object
        Parameters:
            None
        """
        super(UI, self).__init__()

        self.proc = None
        self.proc = self.run()
        if self.proc is None:
            pass

    def run(self):
        """Launches the UI process
        """
        cmd = """gst-switch-ui \
                    """
        with open(os.devnull, 'w') as tempf:
            process = subprocess.Popen(cmd.split(), stdout=tempf, stderr=tempf,  bufsize=-1, shell=False)
        #print "created process:", proc, proc.pid
        return process

    def end(self):
        """Stops the UI
        Returns:
            True on success
            False on failure
        Parameters:
            None
        """
        proc = self.proc
        ret = True
        try:
            proc.terminate()
            print "UI killed"
        except:
            ret = False
        return ret

    def connect_controller(self, Controller=None):
        """Connect the UI to the controller enabling all further method calls
        Creates a Controller() and returns it if it does not exist
        Returns:
            Controller object on success
            None on failure
        Parameters:
            Controller object(optional)
        """
        pass

