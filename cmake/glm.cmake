
include(ExternalProject)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	ExternalProject_Add(
		glm
		URL "https://github.com/johnmcfarlane/glm/archive/a4d22127522833607d646ea3622a3a1e329ea67b.zip"
		URL_MD5 "75339321dd1974e2c5b30b6778d3bbe2"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
else ()
	ExternalProject_Add(
		glm
		GIT_REPOSITORY "https://github.com/johnmcfarlane/glm.git"
		GIT_TAG "heterogeneous-op"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
endif ()

ExternalProject_Get_Property(glm INSTALL_DIR)
set(GLM_INCLUDE_DIR "${INSTALL_DIR}/include")
set(GLM_LIB "${INSTALL_DIR}/lib/glew32s.lib")
