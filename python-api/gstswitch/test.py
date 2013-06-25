from gstswitch import *
from time import sleep
import subprocess
import os

# all executables (gst-launch-1.0, gst-switch-srv, gst-switch-ui, gst-switch-cap) at this path
path = '/home/hyades/gst/master/gstreamer/tools/.libs/' 
os.chdir(path)
s = Server()
s.run()	# launches the server default parameters
try:
	sleep(0.5)
	cmd = path
	# connects a gstreamer module to view the output of the gst-switch-srv
	cmd += "gst-launch-1.0 tcpclientsrc port=3001 ! gdpdepay ! autovideosink"
	proc = subprocess.Popen(cmd.split(),  bufsize=-1, shell=False)
	# adding two test video sources
	s.new_test_video()
	s.new_test_video(clockoverlay=True)
	# waiting till user ends the server
	raw_input()
	s.end()
except:
	# to kill off all processes that are created by the program
	s.brute_end()

