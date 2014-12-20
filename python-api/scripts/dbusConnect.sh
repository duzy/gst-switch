dbus-send \
 	--address="unix:abstract=gstswitch" \
 	--print-reply=literal \
 	--dest="info.duzy.gst_switch.SwitchClientInterface" \
 	/info/duzy/gst_switch/SwitchController \
 	info.duzy.gst_switch.SwitchControllerInterface.get_compose_port

gdbus introspect --address unix:abstract=gstswitch \
				 --dest info.duzy.gst.switch.SwitchUIInterface \
				 --object-path /info/duzy/gst/switch/SwitchUI
