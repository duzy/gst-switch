from gstswitch import *
from time import sleep
import subprocess
s = Server()
sleep(0.5)
u = UI()
s.new_test_video()
sleep(5)
s.new_test_video(clockoverlay=True)
sleep(15)
s.end()
proc.kill()