import os
import sys
import signal
import subprocess

from exception import *
from time import sleep


class BaseServer(object):
    """Class controlling Basic Properties of the Server

    :param: None
    """
    def __init__(self):
        super(BaseServer, self).__init__()

    def set_video_port(self, video_port):
        """Sets the server's video port
        Should be integer between 1 and 65535

        :param video_port: Video port
        :returns: nothing
        """
        try:
            video_port + 1
        except TypeError:
            raise TypeError
        if video_port > 0 and video_port < 65536:
            self.VIDEO_PORT = str(video_port)
        else:
            raise ValueError('video port not in range')

    def set_audio_port(self, audio_port):
        """Sets the server's audio port
        Should be integer between 1 and 65535

        :param audio_port: Audio port
        :returns: nothing
        """
        try:
            audio_port + 1
        except TypeError:
            raise TypeError
        if audio_port > 0 and audio_port < 65536:
            self.AUDIO_PORT = str(audio_port)
        else:
            raise ValueError('audio port not in range')

    def set_control_port(self, control_port):
        """Sets the server's control port
        Should be integer between 1 and 65535

        :param control_port: Control port
        :returns: nothing
        """
        try:
            control_port + 1
        except TypeError:
            raise TypeError
        if control_port > 0 and control_port < 65536:
            self.CONTROL_PORT = str(control_port)
        else:
            raise ValueError('control port not in range')

    def set_record_file(self, record_file):
        """Sets the record file name format - [name-date-time.data]

        :param record_file: The record file name
        :returns: nothing
        """
        if type(record_file) != str:
            raise TypeError('record file name should be string')
        if record_file.find('/') >= 0:
            raise ValueError("record file name should not contain '/'")
        if len(record_file) > 0:
            self.RECORD_FILE = record_file
        else:
            raise ValueError('record file name length should be greater than 0')

    def get_video_port(self):
        """Returns the video port number

        :param: None
        :returns: Video port
        """
        if type(self.VIDEO_PORT) != str:
            raise TypeError("Video port: ", self.VIDEO_PORT, " should be a string")
        try:
            int(self.VIDEO_PORT)
        except:
            raise ValueError("Video port: ", self.VIDEO_PORT, " should have integral value")
        return self.VIDEO_PORT

    def get_audio_port(self):
        """Returns the audio port number

        :param: None
        :returns: Audio port
        """
        if type(self.AUDIO_PORT) != str:
            raise TypeError("audio port: ", self.AUDIO_PORT, " should be a string")
        try:
            int(self.AUDIO_PORT)
        except:
            raise ValueError("audio port: ", self.AUDIO_PORT, " should have integral value")
        return self.AUDIO_PORT

    def get_control_port(self):
        """Returns the control port number

        :param: None
        :returns: Control port
        """
        if type(self.CONTROL_PORT) != str:
            raise TypeError("Control port: ", self.CONTROL_PORT, " should be a string")
        try:
            int(self.CONTROL_PORT)
        except:
            raise ValueError("Control port: ", self.CONTROL_PORT, " should have integral value")
        return self.CONTROL_PORT

    def get_record_file(self):
        """Returns the record file format

        :param: None
        :returns: Record file format
        """
        if type(self.RECORD_FILE) != str:
            raise TypeError("Record File:", self.RECORD_FILE, "should be a string")
        if self.RECORD_FILE.find('/') >= 0:
            raise ValueError("Record file name should not have '/'")
        return self.RECORD_FILE


class ServerProcess(object):
    """Class handling controlling the server process

    :param: None
    """

    def __init__(self):
        super(ServerProcess, self).__init__()
        pass

    def run(self):
        """Launches the server process

        :param: None
        :returns: nothing
        """
        self.proc = None
        self.pid = -1
        print "Starting server"
        self.proc = self._run_process()
        if self.proc is None:
            pass
        else:
            self.pid = self.proc.pid
        # TODO: Sleep time may vary
        sleep(0.5)

    def _run_process(self):
        """Private method for running gst-switch-srv process
        """
        cmd = self.PATH
        # cmd = ''
        cmd += """gst-switch-srv \
                    --video-input-port=%s \
                    --audio-input-port=%s \
                    --control-port=%s \
                    --record=%s """ % (self.VIDEO_PORT, self.AUDIO_PORT, self.CONTROL_PORT, self.RECORD_FILE)
        proc = self._start_process(cmd)
        print "process:", proc
        if proc is None:
            print 'ERROR: Server unable to create process'
            pass
        else:
            print 'Created process with PID:%s', str(proc.pid)
            return proc

    def _start_process(self, cmd):
        """Private method for starting a process

        :param cmd: The command which needs to be excecuted
        :returns: process created
        """
        print 'Creating process %s' % (cmd)
        with open(os.devnull, 'w') as tempf:
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

    def set_executable_path(self, path):
        """Sets the path where all exceutables
        gst-switch-srv, gst-launch-1.0, wtc are located

        :param path: Path where exceutables are present
        :returns: nothing
        """
        self.PATH = path


class Server(BaseServer, ServerProcess):
    """Controls all Server operations

    :param path: Path where all exceutables gst-switch-srv, gst-launch-1.0, etc are located
    :param video_port: The video port number - default = 3000
    :param audio_port: The audio port number - default = 4000
    :param control_port: The control port number - default = 5000
    :param record_file: The record file format
    :returns: nothing
    """

    def __init__(self, path, video_port=3000, audio_port=4000, control_port=5000, record_file='record.data'):
        super(Server, self).__init__()
        self.set_executable_path(path)
        self.set_video_port(video_port)
        self.set_audio_port(audio_port)
        self.set_control_port(control_port)
        self.set_record_file(record_file)
