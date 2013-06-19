from gstswitch import *
from time import sleep
import subprocess

s = Server()
sleep(0.5)
#u = UI()
cmd = "gst-launch-1.0 tcpclientsrc port=3001 ! gdpdepay ! autovideosink"
proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, bufsize=-1, shell=False)
s.new_test_video()
s.new_test_video(clockoverlay=True)
#wait for user to end the server
raw_input()
proc.kill()
s.end()

