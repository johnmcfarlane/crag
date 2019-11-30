
include(ExternalProject)

set(CNL_TAG "ec6c5c4a75920f5ff659abcb364aec88b8170d2c")
set(CNL_PROJECT "https://github.com/johnmcfarlane/cnl")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CNL_URL "${CNL_PROJECT}/archive/${CNL_TAG}.zip")
	ExternalProject_Add(
			cnl
			URL "${CNL_URL}"
			URL_MD5 "${CNL_URL_MD5}"
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
else ()
	ExternalProject_Add(
			cnl
			GIT_REPOSITORY "${CNL_PROJECT}.git"
			GIT_TAG "${CNL_TAG}"
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
endif ()

ExternalProject_Get_Property(cnl INSTALL_DIR)
set(CNL_INCLUDE_DIR "${INSTALL_DIR}/include")
