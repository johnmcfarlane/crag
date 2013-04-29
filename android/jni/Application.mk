# system stlport_static stlport_shared gnustl_static gnustl_shared gabi++_static gabi++_shared none
#APP_STL := system	# fails on <algorithm>
#APP_STL := stlport_static	# fails on <array>
#APP_STL := stlport_shared	# fails on <array>
APP_STL := gnustl_static	# success?
#APP_STL := gnustl_shared	# success?
#APP_STL := gabi++_static	# fails on <algorithm>
#APP_STL := gabi++_shared	# fails on <algorithm>
#APP_STL := none	# fails on <cassert>

NDK_TOOLCHAIN_VERSION := 4.7
APP_PLATFORM := android-10

