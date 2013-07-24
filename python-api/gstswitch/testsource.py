import gi
gi.require_version('Gst', '1.0')
from gi.repository import GObject, Gst

import os
import sys
import random

# from pipeline import *
#IMPORTS

GObject.threads_init()
Gst.init(None)

__all__ = ["Preview", "VideoSrc", ]


class BasePipeline(Gst.Pipeline):
    """A Basic pipeline
    """

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
    """A Video Pipeline which can be used by a video test Source
    """

    def __init__(self, port, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
        super(VideoPipeline, self).__init__()

        self.host = '127.0.0.1'

        src = self.make_videotestsrc(pattern)
        self.add(src)
        vfilter = self.make_capsfilter(width, height)
        self.add(vfilter)
        src.link(vfilter)
        gdppay = self.make_gdppay()
        self.add(gdppay)
        if timeoverlay:
            _timeoverlay = self.make_timeoverlay()
        if clockoverlay:
            _clockoverlay = self.make_clockoverlay()
        if timeoverlay and clockoverlay:
            self.add(_timeoverlay)
            self.add(_clockoverlay)
            vfilter.link(_timeoverlay)
            _timeoverlay.link(_clockoverlay)
            _clockoverlay.link(gdppay)
        if timeoverlay:
            self.add(_timeoverlay)
            vfilter.link(_timeoverlay)
            _timeoverlay.link(gdppay)
        if clockoverlay:
            self.add(_clockoverlay)
            vfilter.link(_clockoverlay)
            _clockoverlay.link(gdppay)
        else:
            vfilter.link(gdppay)

        sink = self.make_tcpclientsink(port)
        self.add(sink)
        gdppay.link(sink)

    def make_videotestsrc(self, pattern):
        element = self.make('videotestsrc', 'src')
        element.set_property('pattern', int(pattern))
        return element

    def make_capsfilter(self, width, height):
        element = self.make("capsfilter", "vfilter")
        capsstring = "video/x-raw, format=(string)I420, width=%s, height=%s" % (str(width), str(height))
        caps = Gst.Caps.from_string(capsstring)
        element.set_property('caps', caps)
        return element

    def make_gdppay(self):
        element = self.make('gdppay', 'gdppay')
        return element

    def make_timeoverlay(self):
        element = self.make('timeoverlay', 'timeoverlay')
        element.set_property('font-desc', "Verdana bold 50")
        return element

    def make_clockoverlay(self):
        element = self.make('clockoverlay', 'clockoverlay')
        element.set_property('font-desc', "Verdana bold 50")
        return element

    def make_tcpclientsink(self, port):
        element = self.make('tcpclientsink', 'tcpclientsink')
        element.set_property('host', self.host)
        element.set_property('port', int(port))
        return element


class PreviewPipeline(BasePipeline):
    """docstring for PreviewPipeline"""
    def __init__(self, port):
        super(PreviewPipeline, self).__init__()
        self.host = '127.0.0.1'
        self.preview_port = port
        src = self.make_tcpclientsrc(port)
        self.add(src)
        gdpdepay = self.make_gdpdepay()
        self.add(gdpdepay)
        src.link(gdpdepay)
        sink = self.make_autovideosink()
        self.add(sink)
        gdpdepay.link(sink)

    def make_tcpclientsrc(self, port):
        element = self.make('tcpclientsrc', 'tcpclientsrc')
        element.set_property('host', self.host)
        element.set_property('port', self.preview_port)
        return element

    def make_gdpdepay(self):
        element = self.make('gdpdepay', 'gdpdepay')
        return element

    def make_autovideosink(self):
        element = self.make('autovideosink', 'autovideosink')
        return element


class VideoSrc(object):
    """A Test Video Source
    """

    def __init__(self, port, width=300, height=200, pattern=None, timeoverlay=False, clockoverlay=False):
        super(VideoSrc, self).__init__()
        self.port = port
        self.width = width
        self.height = height
        self.pattern = self.generate_pattern(pattern)
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

    def generate_pattern(self, pattern):
        """Generates a random patern if not specified
        """
        if pattern is None:
            pattern = random.randint(0, 20)
        pattern = str(pattern)
        return pattern


class Preview(object):
    """docstring for Preview"""
    def __init__(self, port):
        super(Preview, self).__init__()
        self.preview_port = int(port)
        self.pipeline = PreviewPipeline(self.preview_port)

    def run(self):
        self.pipeline.play()

    def pause(self):
        self.pipeline.pause()

    def end(self):
        self.pipeline.disable()
