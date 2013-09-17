
##Writing Tests Using Python-API
###Import the Modules
Ensure that gst-switch/python-api/gstswitch is in PYTHONPATH:
```python
import sys
sys.path.insert(0, install_dir + 'gst-switch/python-api/gstswitch')
# install_dir is the path where the installation started
```
###Start the GstSwitch Server
```python
from gstswitch.server import Server

PATH = '/usr/bin'
# The default location is '/usr/bin'. Change to wherever the gst-switch executables are located
serv = Server(path=PATH, video_port=3000, audio_port=4000)
serv.run() 
```

###Add Some Sources
```python
from gstswitch.helpers import TestSources

sources = TestSources(video_port=video_port, audio_port=audio_port)
sources.new_test_video()
sources.new_test_audio()
```

###Add a Preview to See the Output!!
```python
from gstswitch.helpers import PreviewSinks

preview = PreviewSinks(video_port=3000, audio_port=4000)
preview.run
```

###Remote Method Call Over DBus

####Initialize the Controller
```python
from gstswitch.controller import Controller

controller = Controller()
controller.establish_connection()
```
####Calling Remote Methods
* Get Compose Port: `port = controller.get_compose_port()`
* Get Encode Port: `port = controller.get_encode_port()`
* Get Audio Port: `port = controller.get_audio_port()`
* Get All Preview Ports: `ports = controller.get_preview_ports()`
* Change the PIP Mode: `result = controller.set_composite_mode(mode=1)`
* Move or Adjust the PIP: `result = controller.adjust_pip(xpos=50, ypos=50, 0, 0)`
* Switch the Channel. Channel is specified as `ord('A')`, `ord('a')` or `ord('b')`.  `result = controller.switch(channel=ord('A'), port=3004)`
* Start a New Record: `result = controller.new_record()`

###Terminating the GstSwitch Server
The `server.run()` should always be kept in a try/finally block, so that if any Exception is caught, all processes terminate safely.
i.e.
```python
server = Server(path=PATH, video_port=3000, audio_port=4000)
try:
    server.run()
    # rest of the code comes here
    # ......
    server.terminate()
finally:
    if server.proc:
        server.terminate()
```
`server.terminate()` can be replaced by `server.kill()`. In the latter SIGKILL will be sent to the process.
