"""
The server deals with all operations controlling gst-switch-srv
These include all OS related tasks
"""

import os
import signal
import subprocess

from errno import ENOENT
from .exception import PathError, ServerProcessError
from time import sleep


__all__ = ["Server", ]


TOOLS_DIR = '/'.join(os.getcwd().split('/')[:-1]) + '/tools/'


class Server(object):

    """Control all server related operations

    :param path: Path where all executables
    gst-switch-srv, gst-launch-1.0, etc are located
    :param video_port: The video port number - default = 3000
    :param audio_port: The audio port number - default = 4000
    :param control_port: The control port number - default = 5000
    :param record_file: The record file format
    :returns: nothing
    """
    SLEEP_TIME = 0.5

    def __init__(
            self,
            path,
            video_port=3000,
            audio_port=4000,
            control_port=5000,
            record_file='record.data'):

        super(Server, self).__init__()

        self._path = None
        self._video_port = None
        self._audio_port = None
        self._control_port = None
        self._record_file = None
        self.gst_option_string = None

        self.path = path
        self.video_port = video_port
        self.audio_port = audio_port
        self.control_port = control_port
        self.record_file = record_file

        self.proc = None
        self.pid = -1

    @property
    def path(self):
        """Get the path"""
        return self._path

    @path.setter
    def path(self, path):
        """Set path
        :raises ValueError: Path cannot be left blank
        """
        if not path:
            raise ValueError("Path '{0}' cannot be blank".format(path))
        else:
            self._path = path

    @property
    def video_port(self):
        """Get the video port"""
        return self._video_port

    @video_port.setter
    def video_port(self, video_port):
        """Set Video Port
        :raises ValueError: Video Port cannot be left blank
        :raises ValueError: Video Port must be in range 1 to 65535
        :raises TypeError: Video Port must be a string or a number
        """
        if not video_port:
            raise ValueError("Video Port '{0}' cannot be blank"
                             .format(video_port))
        else:
            try:
                i = int(video_port)
                if i < 1 or i > 65535:
                    raise ValueError('Video Port must be in range 1 to 65535')
                else:
                    self._video_port = video_port
            except TypeError:
                raise TypeError("Video Port must be a string or a number, "
                                "not '{0}'".format(type(video_port)))

    @property
    def audio_port(self):
        """Get the audio port"""
        return self._audio_port

    @audio_port.setter
    def audio_port(self, audio_port):
        """Set Audio Port
        :raises ValueError: Audio Port cannot be left blank
        :raises ValueError: Audio Port must be in range 1 to 65535
        :raises TypeError: Audio Port must be a string or a number
        """
        if not audio_port:
            raise ValueError("Audio Port '{0}' cannot be blank"
                             .format(audio_port))
        else:
            try:
                i = int(audio_port)
                if i < 1 or i > 65535:
                    raise ValueError('Audio Port must be in range 1 to 65535')
                else:
                    self._audio_port = audio_port
            except TypeError:
                raise TypeError("Audio Port must be a string or a number,"
                                " not '{0}'".format(type(audio_port)))

    @property
    def control_port(self):
        """Get the control port"""
        return self._control_port

    @control_port.setter
    def control_port(self, control_port):
        """Set Control Port
        :raises ValueError: Control Port cannot be left blank
        :raises ValueError: Control Port must be in range 1 to 65535
        :raises TypeError: Control Port must be a string or a number
        """
        if not control_port:
            raise ValueError("Control Port '{0}' cannot be blank"
                             .format(control_port))
        else:
            try:
                i = int(control_port)
                if i < 1 or i > 65535:
                    raise ValueError(
                        'Control Port must be in range 1 to 65535')
                else:
                    self._control_port = control_port
            except TypeError:
                raise TypeError("Control Port must be a string or a number,"
                                " not '{0}'".format(type(control_port)))

    @property
    def record_file(self):
        """Get the record file"""
        return self._record_file

    @record_file.setter
    def record_file(self, record_file):
        """Set Record File
        :raises ValueError: Record File cannot be left blank
        :raises ValueError: Record File cannot have forward slashes
        """
        if not record_file:
            raise ValueError("Record File '{0}' cannot be blank"
                             .format(record_file))
        else:
            rec = str(record_file)
            if rec.find('/') < 0:
                self._record_file = rec
            else:
                raise ValueError("Record File: '{0}' "
                                 "cannot have forward slashes".format(rec))

    def run(self, gst_option=''):
        """Launch the server process

        :param: None
        :gst-option: Any gstreamer option.
        Refer to http://www.linuxmanpages.com/man1/gst-launch-0.8.1.php#lbAF.
        Multiple can be added separated by spaces
        :returns: nothing
        :raises IOError: Fail to open /dev/null (os.devnull)
        :raises PathError: Unable to find gst-switch-srv at path specified
        :raises ServerProcessError: Running gst-switch-srv
        gives a OS based error.
        """
        self.gst_option_string = gst_option
        print "Starting server"
        self.proc = self._run_process()
        if self.proc:
            self.pid = self.proc.pid
        sleep(self.SLEEP_TIME)

    def _run_process(self):
        """Non-public method: Runs the gst-switch-srv process
        """
        cmd = self.path
        cmd += """gst-switch-srv \
                    {0} \
                    --video-input-port={1} \
                    --audio-input-port={2} \
                    --control-port={3} \
                    --record={4} """ .format(self.gst_option_string,
                                             self.video_port,
                                             self.audio_port,
                                             self.control_port,
                                             self.record_file)
        cmd = " ".join(cmd.split())
        proc = self._start_process(cmd)
        return proc

    def _start_process(self, cmd):
        """Non-public method: Start a process

        :param cmd: The command which needs to be executed
        :returns: process created
        """
        print 'Creating process %s' % (cmd)
        try:
            with open('server.log', 'w') as tempf:
                process = subprocess.Popen(
                    cmd.split(),
                    stdout=tempf,
                    stderr=tempf,
                    bufsize=-1,
                    shell=False)
                print cmd
                return process
        except OSError as error:
            if error.errno == ENOENT:
                raise PathError("Cannot find gst-switch-srv at path:"
                                " '{0}'".format(self.path))
            else:
                raise ServerProcessError("Internal error "
                                         "while launching process")

    @classmethod
    def make_coverage(cls):
        """Generate coverage
        Calls 'make coverage' to generate coverage in .gcov files
        """
        cmd = 'make -C {0} coverage'.format(TOOLS_DIR)
        print TOOLS_DIR
        with open(os.devnull, 'w'):
            proc = subprocess.Popen(
                cmd.split(),
                bufsize=-1,
                shell=False)
            out, _ = proc.communicate()
            print out

    def terminate(self, cov=False):
        """Terminate the server.
        self.proc is made None on success

        :param: None
        :returns: True when success
        :raises ServerProcessError: Process does not exist
        :raises ServerProcessError: Cannot terminate process. Try killing it
        """
        print 'Killing server'
        proc = self.proc
        if proc is None:
            raise ServerProcessError('Server Process does not exist')
        else:
            try:
                if cov:
                    self.gcov_flush()
                    self.make_coverage()
                proc.terminate()
                print 'Server Killed'
                self.proc = None
                return True
            except OSError:
                raise ServerProcessError("Cannot terminate server process. "
                                         "Try killing it")

    def kill(self, cov=False):
        """Kill the server process by sending signal.SIGKILL
        self.proc is made None on success

        :param: None
        :returns: True when success
        :raises ServerProcessError: Process does not exist
        :raises ServerProcessError: Cannot kill process
        """
        if self.proc is None:
            raise ServerProcessError('Server Process does not exist')
        else:
            try:
                if cov:
                    self.gcov_flush()
                    self.make_coverage()
                os.kill(self.pid, signal.SIGKILL)
                self.proc = None
                return True
            except OSError:
                raise ServerProcessError('Cannot kill process')

    def gcov_flush(self):
        """Generate gcov coverage by sending the signal SIGUSR1
        The generated gcda files are dumped in tools directory.
        Does not kill the process

        :param: None
        :returns: True when success
        :raises ServerProcessError: If Server is not running
        :raises ServerProcessError: Unable to send signal
        """

        if self.proc is None:
            raise ServerProcessError('Server process does not exist')
        else:
            try:
                print "GCOV FLUSH"
                os.kill(self.pid, signal.SIGUSR1)
                return True
            except OSError:
                raise ServerProcessError('Unable to send signal')
