
include(ExternalProject)

set(FIXED_POINT_TAG "660aa93f4201d0bc5a67bc8bc1c6a57c95d1da56")
set(FIXED_POINT_PROJECT "https://github.com/johnmcfarlane/fixed_point")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(FIXED_POINT_URL "${FIXED_POINT_PROJECT}/archive/${FIXED_POINT_TAG}.zip")
	ExternalProject_Add(
			fixed_point
			URL "${FIXED_POINT_URL}"
			URL_MD5 "${FIXED_POINT_URL_MD5}"
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ""
			INSTALL_COMMAND ""
	)
else ()
	ExternalProject_Add(
			fixed_point
			GIT_REPOSITORY "${FIXED_POINT_PROJECT}.git"
			GIT_TAG "${FIXED_POINT_TAG}"
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ""
			INSTALL_COMMAND ""
	)
endif ()

ExternalProject_Get_Property(fixed_point SOURCE_DIR)
set(FIXED_POINT_INCLUDE_DIR "${SOURCE_DIR}/include")
