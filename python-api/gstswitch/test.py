from gstswitch import *
import time

s = Server()
time.sleep(2)

u = UI()
time.sleep(2)

s.new_test_video()
time.sleep(10)
s.new_test_video(clockoverlay=True)
time.sleep(10)
s.end()