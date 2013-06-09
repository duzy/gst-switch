from controller import Controller
from testsource import VideoSource

import os, sys, signal, subprocess
import random
#IMPORTS

class Server(object):
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

		self.video_port = str(video_port)
		self.audio_port = str(audio_port)
		self.control_port = str(control_port)
		self.record_file = record_file
		self.proc = None
		self.Controller = None
		self.TESTS = []

		self.proc = self.run()
		if self.proc == None:
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

	def run(self):
		"""Launches the server
		"""
		cmd = """gst-switch-srv \
					--video-input-port=%s \
					--audio-input-port=%s \
					--control-port=%s \
					--record=%s """ %(self.video_port, self.audio_port, self.control_port, self.record_file)
		print cmd.split()
		proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, bufsize=-1, shell=False)
		print "created process:", proc, proc.pid
		return proc

	def end(self):
		"""Stops the server
		Returns:
			True on success
			False on failure
		Parameters:
			None
		"""
		self.endAllTestVideo()
		proc = self.proc
		ret = True
		try:
			proc.terminate()
			print "server killed"
		except:
			ret = False
		return ret

	def new_test_video(self, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		"""Start a new test source
		"""
		testsrc = TestVideoSrc(self.video_port, width, height, pattern, timeoverlay, clockoverlay)
		if testsrc == None:
			pass
		self.TESTS.append(testsrc)

	def get_test_video(self):
		"""
		"""
		i=0
		for x in self.TESTS:
			print i,"pattern:",x.pattern
			i+=1

	def end_test_video(self, index):
		"""
		"""
		testsrc = self.TESTS[index]
		testsrc.end()
		self.TESTS.remove(self.TESTS[index])

	def endAllTestVideo(self):
		"""
		"""
		for x in range(len(self.TESTS)):
			self.end_test_video(0)


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
		if self.proc == None:
			pass

	def run(self):
		"""Launches the UI process
		"""
		cmd = """gst-switch-ui \
					"""
		proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, bufsize=-1, shell=False)
		print "created process:", proc, proc.pid
		return proc

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



class TestVideoSrc(object):
	"""docstring for TestVideoSrc"""


	def __init__(self, port, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		super(TestVideoSrc, self).__init__()
		"""Contructor for TestVideoSrc class
		Returns:
			None
		Parameters:
			videotestsrc pattern(optional): chooses random if not specified
			#some other optional
		"""

		if timeoverlay:
			self.TIMEOVERLAY = True
		else:
			self.TIMEOVERLAY = False
		if clockoverlay:
			self.CLOCKOVERLAY = True
		else:
			self.CLOCKOVERLAY = False
		self.WIDTH = width
		self.HEIGHT = height
		self.port = port
		self.pattern = self.get_pattern(pattern)

		self.proc = None
		self.proc = self.run()
		if self.proc == None:
			pass
	
	def run(self):
		"""Launches a Test Source

		"""
		
		if self.TIMEOVERLAY == True:
			timeoverlay = """timeoverlay font-desc="Verdana bold 50" ! """
		else:
			timeoverlay = " "
		if self.CLOCKOVERLAY:
			clockoverlay = """clockoverlay font-desc="Verdana bold 50" ! """
		else:
			clockoverlay = " "
		# @TO-DO: do using gi.repository.Gst - gst-launch-1.0 
		cmd = """gst-launch-1.0   \
					videotestsrc pattern=%s ! \
					video/x-raw,width=%s,height=%s ! \
					%s \
					%s \
					gdppay ! \
					tcpclientsink port=%s """ %(self.pattern, self.WIDTH, self.HEIGHT, timeoverlay, clockoverlay, str(self.port))
		print cmd
		proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, bufsize=-1, shell=False)
		print "created process:", proc, proc.pid
		return proc

	def end(self):
		"""Stops the TestVideoSrc
		"""
		proc = self.proc
		ret = True
		try:
			proc.terminate()
			print "TestVideo pattern:%s killed" %(self.pattern)
		except:
			ret = False
		return ret


	def get_pattern(self, pattern):
		"""Generates a random patern if not specified
		"""
		print pattern
		if pattern==None:
			pattern = random.randint(0,20)
		pattern = str(pattern)
		print pattern
		return pattern