#
#  By Duzy Chan <code@duzy.info>, 2012, 2013
#
function do-adlint ()
{
    adlint "$@"
}

function do-splint ()
{
    local stage=$(gst-stage)
    local LINT=splint
    local LINT_PARAMETERS="\
      -I$stage/include/gstreamer-1.0 \
      $(pkg-config --cflags-only-I glib-2.0 gtk+-3.0) \
      "
    $LINT ${LINT_PARAMETERS} "$@"
}

function do-scan-build ()
{
    local stage=$(gst-stage)
    local sb="/store/open/llvm/tools/clang/tools/scan-build/scan-build"
    if [[ ! -f $sb ]]; then
	sb="scan-build"
    fi
    sb="scan-build"
    mkdir -p lint
    $sb ./configure --prefix=$stage
    $sb -v -stats -analyze-headers \
	-enable-checker core.AttributeNonNull \
	-enable-checker core.CallAndMessage \
	-enable-checker core.DivideZero \
	-enable-checker core.NullDereference \
	-enable-checker core.StackAddressEscape \
	-enable-checker core.UndefinedBinaryOperatorResult \
	-enable-checker core.VLASize \
	-enable-checker core.builtin.BuiltinFunctions \
	-enable-checker core.builtin.NoReturnFunctions \
	-enable-checker core.uninitialized.ArraySubscript \
	-enable-checker core.uninitialized.Assign \
	-enable-checker core.uninitialized.Branch \
	-enable-checker core.uninitialized.CapturedBlockVariable \
	-enable-checker core.uninitialized.UndefReturn \
	-enable-checker deadcode.DeadStores \
	-o lint "$@" make

    #-enable-checker core.AdjustedReturnValue \
    #-enable-checker deadcode.IdempotentOperations \
}

function main ()
{
    #do-adlint "$@"
    #do-splint "$@"
    do-scan-build "$@"
}
