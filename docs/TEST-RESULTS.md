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
File 'gstswitchserver.c'
Lines executed:44.44% of 765
gstswitchserver.c:creating 'gstswitchserver.c.gcov'

File 'gstrecorder.c'
Lines executed:70.18% of 114
gstrecorder.c:creating 'gstrecorder.c.gcov'

File 'gstcase.c'
Lines executed:0.00% of 227
gstcase.c:creating 'gstcase.c.gcov'

File 'gio/gsocketinputstream.c'
Lines executed:0.00% of 44
gio/gsocketinputstream.c:creating 'gsocketinputstream.c.gcov'

File 'gstcomposite.c'
Lines executed:60.38% of 371
gstcomposite.c:creating 'gstcomposite.c.gcov'

File 'gstswitchcontroller.c'
Lines executed:67.06% of 340
gstswitchcontroller.c:creating 'gstswitchcontroller.c.gcov'

File 'gstworker.c'
Lines executed:57.47% of 348
gstworker.c:creating 'gstworker.c.gcov'

```

The files which are tested are:
* gstswitchserver.c
* gstrecorder.c
* gstcomposite.c
* gstswitchcontroller.c
* gstworker.c

The files left untested are:
* gstvideodisp.c
* gstswitchclient.c
* gstcase.c
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
