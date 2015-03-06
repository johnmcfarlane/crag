# do not run directly

# crag/nacl
NACL_DIR="$( pwd )"

# crag/nacl/script
SCRIPT_DIR="$NACL_DIR"/script

# something like "pepper_41"
SDK_NAME="$( cat "$NACL_DIR"/bundle_version )"

# directory with NaCL SDK of choice
NACL_SDK_ROOT="$NACL_DIR"/nacl_sdk/"$SDK_NAME"

# used for "make -jN"
NCPUS="1"
if [ -n `which nproc` ]; then
    NCPUS=`nproc`
fi  

echo "script: \"$0\""
echo "bundle: \"$SDK_NAME\""
