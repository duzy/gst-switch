#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./scripts/common.sh
. ./scripts/launcher.sh

function main()
{
    launch -v \
	filesrc location=\"$PWD/$1\" ! avidemux name=d \
	d. ! queue ! faad ! audioconvert ! alsasink \
	d. ! queue ! vp8dec ! videoconvert ! videoscale ! ximagesink
}
