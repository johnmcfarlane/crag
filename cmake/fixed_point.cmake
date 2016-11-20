
include(ExternalProject)

set(FIXED_POINT_TAG "0e5dcfd3c9d7e884e74ec8a74a445d1f6485f896")
set(FIXED_POINT_PROJECT "https://github.com/johnmcfarlane/fixed_point")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(FIXED_POINT_URL "${FIXED_POINT_PROJECT}/archive/${FIXED_POINT_TAG}.zip")
	ExternalProject_Add(
			fixed_point
			URL "${FIXED_POINT_URL}"
			URL_MD5 "${FIXED_POINT_URL_MD5}"
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
else ()
	ExternalProject_Add(
			fixed_point
			GIT_REPOSITORY "${FIXED_POINT_PROJECT}.git"
			GIT_TAG "${FIXED_POINT_TAG}"
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
endif ()

ExternalProject_Get_Property(fixed_point INSTALL_DIR)
set(FIXED_POINT_INCLUDE_DIR "${INSTALL_DIR}/include")
