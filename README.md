# Build Status
[![Build Status](https://travis-ci.org/hyades/gst-switch.png?branch=master)](https://travis-ci.org/hyades/gst-switch)
[![Coverage Status](https://coveralls.io/repos/hyades/gst-switch/badge.png)](https://coveralls.io/r/hyades/gst-switch)

# gst-switch
## Purpose
This project is intended to be a replacement of DV-switch based on GStreamer.
##Installing gst-switch
```bash
wget https://raw.github.com/hyades/gst-switch/master/scripts/install2.sh
chmod +x install2.sh
./install2.sh
```
The dependencies are taken care by the installation method.
## Components
gst-switch contains two parts, *gst-switch-srv* and *gst-switch-ui*
## Quick Manual
### The gst-switch Server
The gst-switch server will open at least three ports for video/audio input, and
command controls.
Start gst-switch server as:
```bash
gst-switch-srv --video-input-port=3000 --audio-input-port=4000 --control-port=5000
```
#### Video Input
The default video input port is *3000*. Supported input video format: I420
(video/x-raw), 1280x720 (for debug mode, the video size could be 300x200).
#### Audio Input Port
The default audio input port is *4000*.
#### Control Port
The default command control port is *5000*.

###The gst-switch UI
The gst-switch UI is a graphical controller for controlling the input streams
```bash
gst-switch-ui
```

#### Controls
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

###The gst-switch Capture
Implements speaker tracking
```bash
gst-switch-cap --device='/dev/ttyUSB0' --protocol='visca'
```

###The gst-switch PTZ
Implements Pan-Tilt-Zoom Camera
```bash
gst-switch-ptz --device='/dev/ttyUSB0' --protocol='visca' --video='/dev/video0'
```
