import os
import sys
import signal
import subprocess

from exception import *
from controller import Controller
from time import sleep


class BaseServer(object):
    """Properties of the Server
    """
    def __init__(self):
        super(BaseServer, self).__init__()

    def set_video_port(self, video_port):
        self.VIDEO_PORT = str(video_port)

    def set_audio_port(self, audio_port):
        self.AUDIO_PORT = str(audio_port)

    def set_control_port(self, control_port):
        self.CONTROL_PORT = str(control_port)

    def set_record_file(self, record_file):
        self.RECORD_FILE = record_file

    def get_video_port(self):
        return self.VIDEO_PORT

    def get_audio_port(self):
        return self.AUDIO_PORT

    def get_control_port(self):
        return self.CONTROL_PORT

    def get_record_file(self):
        return self.RECORD_FILE


class ServerProcess(object):
    """Handles all processes created. This includes the server process
    and test sources added to the gst-switch-srv
    """

    def __init__(self):
        super(ServerProcess, self).__init__()
        pass

    def run(self):
        """Launches the server
        """
        self.proc = None
        self.pid = -1
        print "Starting server"
        self.proc = self.run_process()
        if self.proc is None:
            pass
        else:
            self.pid = self.proc.pid
        sleep(0.5)

    def run_process(self):
        cmd = self.PATH
        # cmd = ''
        cmd += """gst-switch-srv \
                    --video-input-port=%s \
                    --audio-input-port=%s \
                    --control-port=%s \
                    --record=%s """ % (self.VIDEO_PORT, self.AUDIO_PORT, self.CONTROL_PORT, self.RECORD_FILE)
        proc = self.start_process(cmd)
        print "process:", proc
        if proc is None:
            print 'ERROR: Server unable to create process'
            pass
        else:
            print 'Created process with PID:%s', str(proc.pid)
            return proc

    def start_process(self, cmd):
        print 'Creating process %s' % (cmd)
        with open(os.devnull, 'w') as tempf:
            process = subprocess.Popen(cmd.split(), stdout=tempf, stderr=tempf,  bufsize=-1, shell=False)
        return process

    def terminate(self):
        """Stops the server
        Returns:
            True on success
            False on failure
        Parameters:
            None
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
        os.kill(self.pid, signal.SIGKILL)

    def set_executable_path(self, path):
        self.PATH = path


class Server(BaseServer, ServerProcess):
    """Controls all Server operations and Test Video Sources
    """

    def __init__(self, path, video_port=3000, audio_port=4000, control_port=5000, record_file='record.data'):
        """Contructor for the Server class
        Returns:
            Server() object
        Parameters:
            video_port(optional)
            audio_port(optional)
            control_port(optional)
            record_file(optional)

        """
        super(Server, self).__init__()
        self.set_executable_path(path)
        self.set_video_port(video_port)
        self.set_audio_port(audio_port)
        self.set_control_port(control_port)
        self.set_record_file(record_file)
