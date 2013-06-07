gdbus call --address unix:abstract=gstswitch \
        --dest info.duzy.gst_switch.SwitchClientInterface \
        --object-path /info/duzy/gst_switch/SwitchController \
        --method info.duzy.gst_switch.SwitchControllerInterface.switch \
        1 \
        3001