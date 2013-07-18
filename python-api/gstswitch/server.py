import os
import sys
import signal
import subprocess

from exception import *
from time import sleep


class Server(object):
    """Controls all Server operations

    :param path: Path where all exceutables gst-switch-srv, gst-launch-1.0, etc are located
    :param video_port: The video port number - default = 3000
    :param audio_port: The audio port number - default = 4000
    :param control_port: The control port number - default = 5000
    :param record_file: The record file format
    :returns: nothing
    """
    SLEEP_TIME = 0.5

    def __init__(self, path, video_port=3000, audio_port=4000, control_port=5000, record_file='record.data'):
        super(Server, self).__init__()
        self.path = path
        self.video_port = video_port
        self.audio_port = audio_port
        self.control_port = control_port
        self.record_file = record_file

    def run(self, gst_option=''):
        """Launches the server process

        :param: None
        :returns: nothing
        """
        self.proc = None
        self.pid = -1
        self.gst_option_string = gst_option
        print "Starting server"
        self.proc = self._run_process()
        self.pid = self.proc.pid
        # TODO: Sleep time may vary
        sleep(self.SLEEP_TIME)

    def _run_process(self):
        """Private method for running gst-switch-srv process
        """
        cmd = self.path
        # cmd = ''
        cmd += """gst-switch-srv \
                    %s \
                    --video-input-port=%s \
                    --audio-input-port=%s \
                    --control-port=%s \
                    --record=%s """ % (self.gst_option_string,
                                       self.video_port,
                                       self.audio_port,
                                       self.control_port,
                                       self.record_file)
        proc = self._start_process(cmd)
        if proc is None:
            raise RuntimeError("unable to create process: %s" % (proc))
        else:
            return proc

    def _start_process(self, cmd):
        """Private method for starting a process

        :param cmd: The command which needs to be excecuted
        :returns: process created
        """
        print 'Creating process %s' % (cmd)
        tempf = open(os.devnull, 'w')
        process = subprocess.Popen(cmd.split(), stdout=tempf, stderr=tempf,  bufsize=-1, shell=False)
        return process

    def terminate(self):
        """Terminates the server

        :param: None
        :returns: True when success
        """
        print 'Killing server'
        proc = self.proc
        ret = True
        try:
            proc.terminate()
            print 'Server Killed'
        except:
            print 'Error killing server'
            ret = False
        return ret

    def kill(self):
        """Kills the server process by sending SIGKILL

        :param: None
        :returns: nothing
        """
        os.kill(self.pid, signal.SIGKILL)
