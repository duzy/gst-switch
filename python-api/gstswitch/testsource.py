import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst

import os, sys, random

#IMPORTS

GObject.threads_init()
Gst.init(None)




class BasePipeline(Gst.Pipeline):
	"""docstring for BasePipeline"""


	def __init__(self):
		super(BasePipeline, self).__init__()
		Gst.Pipeline.__init__(self)
		self._playing = False

	def play(self):
		self._playing = True
		self.set_state(Gst.State.PLAYING)

	def pause(self):
		self._playing = False
		self.set_state(Gst.State.PAUSED)

	def disable(self):
		self._playing = False
		self.set_state(Gst.State.NULL)

	def make(self, elem, description=''):
		element = Gst.ElementFactory.make(elem, description)
		return element



class VideoPipeline(BasePipeline):
	"""docstring for VideoPipeline"""


	def __init__(self, port, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		super(VideoPipeline, self).__init__()
		pattern = self.get_pattern(pattern)

		src = self.make_videotestsrc()
		src.set_property('pattern', int(pattern))
		self.add(src)		
		vfilter = self.make_capsfilter()
		capsstring = "video/x-raw,width=%s,heigth=%s" %(str(width), str(height))
		caps = Gst.caps_from_string(capsstring)
		vfilter.props.caps = caps
		self.add(vfilter)
		src.link(vfilter)
		gdppay = self.make_gdppay()
		if timeoverlay:
			_timeoverlay = self.make_timeoverlay()
			_timeoverlay.set_property('font-desc', "Verdana bold 50")
		if clockoverlay:
			_clockoverlay = self.make_clockoverlay()
			_clockoverlay.set_property('font-desc', "Verdana bold 50")
		if timeoverlay and clockoverlay:
			self.add(_timeoverlay)
			self.add(_clockoverlay)
			self.add(gdppay)
			vfilter.link(_timeoverlay)
			_timeoverlay.link(_clockoverlay)
			_clockoverlay.link(gdppay)
		if timeoverlay:
			self.add(_timeoverlay)
			self.add(gdppay)
			vfilter.link(_timeoverlay)
			_timeoverlay.link(gdppay)
		if clockoverlay:
			self.add(_clockoverlay)
			self.add(gdppay)
			vfilter.link(_clockoverlay)
			_clockoverlay.link(gdppay)
		else:
			self.add(gdppay)
			vfilter.link(gdppay)

		sink = self.make_tcpclientsink()
		sink.set_property('port', int(port))
		self.add(sink)
		gdppay.link(sink)

	def make_videotestsrc(self):
		element = self.make('videotestsrc','src')
		return element

	def make_capsfilter(self):
		element = self.make("capsfilter", "vfilter")
		return element

	def make_gdppay(self):
		element = self.make('gdppay','gdppay')
		return element

	def make_timeoverlay(self):
		element = self.make('timeoverlay','timeoverlay')
		return element

	def make_clockoverlay(self):
		element = self.make('clockoverlay','clockoverlay')
		return element

	def make_tcpclientsink(self):
		element = self.make('tcpclientsink','tcpclientsink')
		return element

	def get_pattern(self, pattern):
		"""Generates a random patern if not specified
		"""
		if pattern==None:
			pattern = random.randint(0,20)
		pattern = str(pattern)
		print pattern
		return pattern
		
class VideoSrc(object):
	"""docstring for VideoSrc"""
	def __init__(self, port, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		super(VideoSrc, self).__init__()
		self.port = port
		self.width = width
		self.height = height
		self.pattern = pattern
		self.timeoverlay = timeoverlay
		self.clockoverlay = clockoverlay

		self.pipeline = VideoPipeline(self.port, self.width, self.height, self.pattern, self.timeoverlay, self.clockoverlay)
		self.run()

	def run(self):
		self.pipeline.play()

	def pause(self):
		self.pipeline.pause()

	def end(self):
		self.pipeline.disable()



		
