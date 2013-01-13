#!/bin/bash
applet=$(basename $0)
script=./scripts/"$applet"_funs.sh
if [[ -f $script ]]; then
    . $script && main $@
else
    echo "Applet $applet not found!"
fi
