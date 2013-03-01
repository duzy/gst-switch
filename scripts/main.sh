#!/bin/bash
#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  

applet=$(basename $0)
script=./scripts/apps/app_$applet.sh
if [[ -f $script ]]; then
    . ./scripts/common.sh
    . $script && main "$@"
else
    echo "Applet $applet not found!"
fi
