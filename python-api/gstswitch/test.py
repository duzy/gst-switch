from gstswitch import *
import time

s = Server()
time.sleep(2)

u = UI()
time.sleep(2)

s.newTestVideo()
time.sleep(10)
s.newTestVideo(clockoverlay=True)
time.sleep(10)
s.end()