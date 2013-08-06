from exception import RangeError
from testsource import Preview, VideoSrc
import pytest
from mock import Mock, patch


class TestVideoSrcPort(object):

	def test_blank(self):
		tests = ['', None, [], {}]
		for test in tests:
			with pytest.raises(ValueError):
				VideoSrc(port=test)

	def test_range(self):
		tests = [-100, 1e7, 65536]
		for test in tests:
			with pytest.raises(RangeError):
				VideoSrc(port=test)

	def test_invalid(self):
		tests = [[1, 2, 3, 4], {1: 2, 2: 3}, '1e10']
		for test in tests:
			with pytest.raises(TypeError):
				VideoSrc(port=test)

	def test_normal(self):
		tests = [1, 65535, 1000]
		for test in tests:
			src = VideoSrc(port=test)
			assert src.port == test

class TestVideoSrcWidth(object):

	def test_blank(self):
		tests = ['', None, [], {}]
		port = 1000
		for test in tests:
			with pytest.raises(ValueError):
				VideoSrc(port=port, width=test)

	def test_non_positive(self):
		tests = ['-1.111', -1.111, 0, -1e10]
		port = 1000
		for test in tests:
			with pytest.raises(ValueError):
				VideoSrc(port=port, width=test)

	def test_invalid(self):
		tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
		port = 1000
		for test in tests:
			with pytest.raises(TypeError):
				VideoSrc(port=port, width=test)

	def test_normal(self):
		tests = [  1e6, 300, '200']
		port = 1000
		for test in tests:
			src = VideoSrc(port=port, width=test)
			assert src.width == test


class TestVideoSrcHeight(object):

	def test_blank(self):
		tests = ['', None, [], {}]
		port = 1000
		for test in tests:
			with pytest.raises(ValueError):
				VideoSrc(port=port, height=test)

	def test_non_positive(self):
		tests = ['-1.111', -1.111, 0, -1e10]
		port = 1000
		for test in tests:
			with pytest.raises(ValueError):
				VideoSrc(port=port, height=test)

	def test_invalid(self):
		tests = [[1, 2, 3], {1: 2, 2: 3}, (1, 2)]
		port = 1000
		for test in tests:
			with pytest.raises(TypeError):
				VideoSrc(port=port, height=test)

	def test_normal(self):
		tests = [1e6, 300, '200']
		port = 1000
		for test in tests:
			src = VideoSrc(port=port, height=test)
			assert src.height == test


class TestVideoSrcPattern(object):

	def test_range(self):
		tests = [-100, 1e7, 65536, -1, 20]
		port = 1000
		for test in tests:
			with pytest.raises(RangeError):
				VideoSrc(port=port, pattern=test)

	def test_invalid(self):
		tests = [[1, 2, 3, 4], {1: 2, 2: 3}]
		port = 1000
		for test in tests:
			with pytest.raises(TypeError):
				VideoSrc(port=port, pattern=test)

	def test_normal(self):
		tests = [1, 0, 19, 10]
		port = 1000
		for test in tests:
			src = VideoSrc(port=port, pattern=test)
			assert src.pattern == str(test)