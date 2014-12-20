##Coverage
The Coverage reports can be found at [coveralls.io](https://coveralls.io/r/hyades/gst-switch). The build is currently failing, therefore the coverage of the integration tests and the c code coverage cannot be found in the coveralls report. The coverage reports are generated for the inetgration test and C code. They are not reported to coveralls.
The coverage are present at the build page of travis.

##Python Integration Tests Coverage
The current coverage reports are:
```bash
Name                   Stmts   Miss  Cover
------------------------------------------
gstswitch/__init__         0      0   100%
gstswitch/connection     183     67    63%
gstswitch/controller     163     41    75%
gstswitch/exception       15      0   100%
gstswitch/helpers        129     44    66%
gstswitch/server         143     36    75%
gstswitch/testsource     312     64    79%
------------------------------------------
TOTAL                    945    252    73%
```

##C Coverage
The C Coverage is:
```bash
File 'gstcase.c'
Lines executed:76.65% of 227
Creating 'gstcase.c.gcov'

File 'gstcomposite.c'
Lines executed:63.34% of 371
Creating 'gstcomposite.c.gcov'

File 'gstrecorder.c'
Lines executed:70.18% of 114
Creating 'gstrecorder.c.gcov'

File 'gstswitchcontroller.c'
Lines executed:70.12% of 338
Creating 'gstswitchcontroller.c.gcov'

File 'gstswitchserver.c'
Lines executed:69.93% of 765
Creating 'gstswitchserver.c.gcov'

File 'gstworker.c'
Lines executed:66.67% of 348
Creating 'gstworker.c.gcov'
```

The files which are tested are:
* gstswitchserver.c
* gstrecorder.c
* gstcomposite.c
* gstswitchcontroller.c
* gstworker.c
* gstcase.c

The files left untested are:
* gstvideodisp.c
* gstswitchclient.c
* gstswitchptz.c
* gstswitchcapture.c
* gstswitchui.c
* gstswitchclient.c
* gstaudiovisual.c

##Untested
The gst-switch controller can have three roles:
* UI
* Capture
* None

The Python-API communicates with the gst-switch controller over DBus. Hence the role that is selected is None. The methods which use the roles UI and Capture are not tested by the API.

The file gstswitchui.c handles the gst-switch-ui.
The file gstswitchcapture.c handles the gst-switch-cap.
The file gstswitchptz.c handles the gst-switch-ptz.
Since the above executables are not executed by the API, they are left untested.

The files gstvideodisp.c and gstaudiodisp.c handle the video/audio contain functions dealing with the gst-switch-ui.

The gstswitchclient.c is also used by gst-switch-ui and hence remains untested.
