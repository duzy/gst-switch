from controller import Controller

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
			

	def connectController(self, Controller=None):
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
		cmd = """gst-switch-srv -v \
					--gst-debug-no-color \
					--video-input-port=%s \
					--audio-input-port=%s \
					--control-port=%s \
					--record=%s """ %(self.video_port, self.audio_port, self.control_port, self.record_file)
		proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=-1, shell=True)
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
		proc = self.proc
		ret = True
		try:
			proc.terminate()
			print "server killed"
		except:
			ret = False
		return ret

	def newTestVideoSrc(self, pattern=None):
		"""Start a new test source
		"""
		testsrc = TestVideoSrc(self.video_port, pattern)
		if testsrc == None:
			pass
		self.TESTS.append(testsrc)

	def getTestVideoSrc(self):
		"""
		"""
		i=1
		for x in self.TESTS:
			print i,"pattern:",x.pattern
			i+=1

	def endTestVideoSrc(self, index):
		"""
		"""
		testsrc = self.TESTS[index]
		testsrc.end()
		self.TESTS.remove(testsrc)



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
		cmd = """gst-switch-ui -v \
					--gst-debug-no-color """
		proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=-1, shell=True)
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

	def connectController(self, Controller=None):
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

	def __init__(self, port, pattern=None):
		super(TestVideoSrc, self).__init__()
		"""Contructor for TestVideoSrc class
		Returns:
			None
		Parameters:
			videotestsrc pattern(optional): chooses random if not specified
		"""
		self.TIMEOVERLAY = True
		self.WIDTH=640
		self.HEIGHT=480

		self.proc = None
		if pattern == None:
			self.pattern = None
		else:
			self.pattern = pattern
		self.proc = self.run(port, self.pattern)
		if self.proc == None:
			pass
	
	def run(self, port, pattern=None):
		"""Launches a Test Source

		"""
		self.pattern = self.getPattern(pattern)
		if self.TIMEOVERLAY == True:
			timeoverlay = """timeoverlay font-desc="Verdana bold 50" ! """
		else:
			timeoverlay = " "
		cmd = """gst-launch-1.0   \
					videotestsrc pattern=%s ! \
					video/x-raw,width=%s,height=%s ! \
					%s \
					gdppay ! \
					tcpclientsink port=%s """ %(self.pattern, self.WIDTH, self.HEIGHT, timeoverlay, str(port))
		print cmd
		proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=-1, shell=True)
		print "created process:", proc, proc.pid
		return proc

	def end(self):
		"""Stops the TestVideoSrc
		"""
		proc = self.proc
		ret = True
		try:
			proc.terminate()
			print "VideoTestSrc killed"
		except:
			ret = False
		return ret


	def getPattern(self, pattern=None):
		if pattern==None:
			pattern = random.randint(0,20)
		pattern = str(pattern)
		return pattern