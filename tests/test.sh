#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#  
. ./scripts/common.sh
. ./scripts/launcher.sh

function check_file() {
    echo "=================================================="
    echo "GOT:"
    cat $1
    echo "--------------------------------------------------"
    echo "EXPECT:"
    echo -n "$2"
    echo "=================================================="
}
