#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./scripts/launcher.sh

function main()
{
    local out="$PWD/$(basename "$1" .data)"
    mkdir -p "$out" && launch -v \
	filesrc location=\"$PWD/$1\" ! avidemux name=d \
	d. ! queue ! vp8dec ! videoconvert ! pngenc ! multifilesink \
	location=\"$out/frame-%08d.png\"
    echo "====================" && ls -l "$out"
}
