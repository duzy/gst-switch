import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst

import os, sys

#IMPORTS

GObject.threads_init()
Gst.init(None)

class _BasePipeline(Gst.Pipeline):
	"""docstring for _BasePipeline"""
	def __init__(self):
		super(_BasePipeline, self).__init__()
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


class VideoPipeline(_BasePipeline):
	"""docstring for VideoPipeline"""
	def __init__(self, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
		super(VideoPipeline, self).__init__()
		pattern = self.get_pattern(pattern)
		if timeoverlay:
			self.timeoverlaystring = """,timeoverlay font-desc="Verdana bold 50"  """
		else: 
			self.timeoverlaystring = ""
		if clockoverlay:
			self.clockoverlaystring = """,clockoverlay font-desc="Verdana bold 50"  """
		else:
			self.clockoverlaystring = ""

		self.capsstring = "video/x-raw, width=%s, heigth=%s %s %s" %(str(width), str(heigth), timeoverlaystring, clockoverlaystring)

		self.src = Gst.ElementFactory.make('videotestsrc','src')
		self.src.set_property('pattern', pattern)
		self.vfilter = gst.element_factory_make("capsfilter", "vfilter")
		self.vfilter.set_property('caps',Gst.caps_from_string(self.capsstring))
		

	def get_pattern(self, pattern):
		"""Generates a random patern if not specified
		"""
		print pattern
		if pattern==None:
			pattern = random.randint(0,20)
		pattern = str(pattern)
		print pattern
		return pattern
		

		