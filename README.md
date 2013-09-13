# Build Status
[![Build Status](https://travis-ci.org/hyades/gst-switch.png?branch=master)](https://travis-ci.org/hyades/gst-switch)
[![Coverage Status](https://coveralls.io/repos/hyades/gst-switch/badge.png?branch=master)](https://coveralls.io/r/hyades/gst-switch?branch=master)

# GstSwitch
## Purpose
This project is intended to be a replacement of DV-switch base on GStreamer.
## Components
GstSwitch contains two parts, *gst-switch-srv* and *gst-switch-ui*
## Quick Manual
### The GstSwitch Server
The GstSwitch server will open at least three ports for video/audio input, and
command controls.
#### Video Input
The video input port is *3000*. Supported input video format: I420
(video/x-raw), 1280x720 (for debug mode, the video size could be 300x200).
#### Audio Input Port
The audio input port is *4000*.
#### Control Port
The command control port is *5000*.
### Controls
<table>
 <tr><td>Key Bindings</td><td>Function</td></tr>

 <tr><td>Ctrl+Arrow</td><td>
 Adjust PIP position.
 </td></tr>

 <tr><td>Ctrl+Shift+Arrow</td><td>
 Adjust PIP size.
 </td></tr>

 <tr><td>Arrow Up/Down</td><td>
 Change selection of video previews. Selecting a video is the first step for
 switching video/audio input.
 </td></tr>

 <tr><td>A</td><td>
 Switch video input for channel A or activate audio input to the selected
 video/audio.
 </td></tr>

 <tr><td>B</td><td>
 Switch video input for channel B to the selected video.
 </td></tr>

 <tr><td>Tab</td><td>
 Change a mode of A/B compositing, stroking it repeatly will cycle within modes
 and off.
 </td></tr>

 <tr><td>Esc</td><td>
 Turn off A/B videos compositing.
 </td></tr>

 <tr><td>r</td><td>
 Start a new recording, the *--record _name_* will be used as a template,
 e.g. *record 2013-01-23 131139.dat*
 </td></tr>
</table>
##Installing GstSwitch
```bash
git clone https://github.com/hyades/gst-switch.git
cd gst-switch
./.travis-setup.sh
```
or you can do
```bash
wget https://raw.github.com/hyades/gst-switch/master/scripts/install.sh
chmod +x install.sh
./install.sh
```
The dependencies are taken care by the installation method.

##Running Tests
```bash
cd python-api
```

####Run Unittests
```bash
make unittests
```

####Run Integration Tests
```bash
make integration
```

####Lint and pep8 Tests
```bash
make lint
make pep8
```
####Running everything together
```bash
make test
```

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

PATH = '/usr/local/bin'
# The default location is '/usr/local/bin'. Change to wherever the gst-switch executables are located
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
* Change the PIP Mode: `result = controller.set_composite_mode(mode=1)
* Move or Adjust the PIP: `result = controller.adjust_pip(xpos=50, ypos=50, 0, 0)
* Switch the Channel. Channel is specified as `ord('A')`, `ord('a')` or `ord('b')`.  `result = controller.switch(channel=ord('A'), port=3004)`
* Start a New Record: `result = controller.new_record()`

