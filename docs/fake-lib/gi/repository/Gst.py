threads_init = lambda *a, **k: None

def init(a):
	pass

class Source(object):
	pass

class Pipeline(object):
	pass

class ElementFactory(object):
	def make(self, a, b):
		pass

class IOCondition:
  IN = 1
  ERR = 8
  HUP = 16
