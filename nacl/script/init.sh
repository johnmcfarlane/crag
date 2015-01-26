# do not run directly
LOCAL_DIR="$( dirname "$0" )"
NACL_DIR="$( cd "$LOCAL_DIR"/..; pwd )"
SDK_NAME="$( cat "$NACL_DIR"/bundle_version )"
NACL_SDK_ROOT="$NACL_DIR"/nacl_sdk/"$SDK_NAME"

NCPUS="1"
if [ -n `which nproc` ]; then
    NCPUS=`nproc`
fi  

echo "script: \"$0\""
echo "bundle: \"$SDK_NAME\""
