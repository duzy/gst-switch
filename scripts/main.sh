#!/bin/bash
#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  

applet=$(basename $0)
script=./scripts/app_$applet.sh
if [[ -f $script ]]; then
    . $script && main "$@"
else
    echo "Applet $applet not found!"
fi
