from gstswitch import *
from time import sleep
import subprocess
import os

path = '/home/hyades/gst/master/gstreamer/tools/.libs/'
os.chdir(path)
s = Server()
s.run()
try:
	
	sleep(2)
	# u = UI()
	cmd = path
	cmd += "gst-launch-1.0 tcpclientsrc port=3001 ! gdpdepay ! autovideosink"
	proc = subprocess.Popen(cmd.split(),  bufsize=-1, shell=False)
	s.new_test_video()
	s.new_test_video(clockoverlay=True)
	#wait for user to end the server
	#sleep(10)
	# u.end()
	raw_input()
	s.end()
except:
	s.brute_end

