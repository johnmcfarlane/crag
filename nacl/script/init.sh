echo init: $0

SDK_NAME=pepper_39
LOCAL_DIR="$( dirname "$0" )"
NACL_DIR="$( cd "$LOCAL_DIR"/..; pwd )"
NACL_SDK_ROOT="$NACL_DIR"/nacl_sdk/"$SDK_NAME"

NCPUS="1"
if [ -n `which nproc` ]; then
    NCPUS=`nproc`
fi  

