import os
import sys
import signal
import subprocess
import logging

from exception import *
from controller import Controller
from testsource import VideoSrc

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class BaseServer(object):
	"""docstring for BaseServer"""
	def __init__(self):
		super(BaseServer, self).__init__()
		
	def set_video_port(self, video_port):
		self.video_port = str(video_port)

	def set_audio_port(self, audio_port):
		self.audio_port = str(audio_port)

	def  set_control_port(self, control_port):
		self.control_port = str(control_port)

	def set_record_file(self, record_file):
		self.record_file = record_file

	def get_video_port(self):
		return self.video_port

	def get_audio_port(self):
		return self.audio_port

	def get_control_port(self):
		return self.control_port

	def  get_record_file(self):
		return self.record_file




class ServerDBusController(object):
	"""docstring for ServerDBusController"""

	def __init__(self):
		super(ServerDBusController, self).__init__()
		
	def initialize_controller():
		pass

	def connect_controller(self, Controller=None):
		"""Connects the Server() to the gdbus enabling all further method calls. 
		Creates Controller and returns it if it does not exist
		Returns:
			Controller object on success
			None object on failure
		Parameters:
			None
		"""
		pass	


		
class ServerTestSourceController(object):
	"""docstring for ServerTestSourceController"""

	def __init__(self):
		super(ServerTestSourceController, self).__init__()
		self.TESTS = []

		

	def new_test_video(self, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		"""Start a new test source
		"""
		logging.info('Adding new test video source')
		testsrc = VideoSrc(self.video_port, width, height, pattern, timeoverlay, clockoverlay)
		if testsrc == None:
			pass
		self.TESTS.append(testsrc)

	def get_test_video(self):
		"""Returns a list of processes acting as test inputs
		"""
		i=0
		for x in self.TESTS:
			print i,"pattern:",x.pattern
			i+=1
		return self.TESTS

	def end_test_video(self, index):
		"""
		"""
		testsrc = self.TESTS[index]
		print 'End source with pattern %s' %(str(testsrc.get_pattern()))
		logging.info('End source with pattern %s' %(str(testsrc.get_pattern())))
		testsrc.end()
		self.TESTS.remove(self.TESTS[index])

	def endAllTestVideo(self):
		"""
		"""
		print 'TESTS:', self.TESTS
		for x in range(len(self.TESTS)):
			self.end_test_video(0)

		
class ServerProcess(ServerTestSourceController):
	"""docstring for ServerProcess"""

	def __init__(self):
		super(ServerProcess, self).__init__()
		pass

	def run(self):
		"""Launches the server
		"""
		self.proc = None
		self.pid = -1
		logging.info('Starting server')
		print "running"
		self.proc = self.run_process()
		if self.proc == None:
			pass
		else:
			self.pid = self.proc.pid

	def run_process(self):
		# cmd = self.PATH
		cmd = ''
		cmd += """gst-switch-srv \
					--video-input-port=%s \
					--audio-input-port=%s \
					--control-port=%s \
					--record=%s """ %(self.video_port, self.audio_port, self.control_port, self.record_file)
		proc = self.start_process(cmd)
		print "process:", proc
		if proc == None:
			logging.error('Error running server')
			pass
		else:
			logging.info('Created process with PID:%s', str(proc.pid))
			return proc

	def start_process(self, cmd):
		logging.info('Creating process %s' %(cmd))
		with open(os.devnull, 'w') as tempf:
			process = subprocess.Popen(cmd.split(), stdout=tempf, stderr=tempf,  bufsize=-1, shell=False)
		return process

	def end(self):
		"""Stops the server
		Returns:
			True on success
			False on failure
		Parameters:
			None
		"""
		print 'Killing server'
		logging.info('Killing server')
		self.endAllTestVideo()
		proc = self.proc
		ret = True
		try:
			proc.terminate()
			logging.info('Server killed')
		except:
			logging.info('Error killing server')
			ret = False
		return ret

	def brute_end(self):
		os.kill(self.pid, signal.SIGKILL)

	def set_executable_path(self, path):
		self.PATH = path



class Server(BaseServer, ServerProcess, ServerDBusController):
	"""docstring for Server"""
	
	def __init__(self, video_port=3000, audio_port=4000, control_port=5000, record_file='record.data'):
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
		#self.set_executable_path(path)
		self.set_video_port(video_port)
		self.set_audio_port(audio_port)
		self.set_control_port(control_port)
		self.set_record_file(record_file)


	

	