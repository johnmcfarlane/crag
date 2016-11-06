
include(ExternalProject)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	ExternalProject_Add(
			glew
			URL "https://github.com/nigels-com/glew/releases/download/glew-2.0.0/glew-2.0.0-win32.zip"
			URL_MD5 "7a11b325d21b610c8ded673719740619"
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ""
			INSTALL_COMMAND ""
	)

	if ("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 14 2015 Win64")
		set(GLEW_ARCHITECTURE "x64")
	elseif ("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 14 2015")
		set(GLEW_ARCHITECTURE "Win32")
	else ()
		message(FATAL_ERROR "Unrecognized MSVC generator, ${CMAKE_GENERATOR}")
	endif ()

	ExternalProject_Get_Property(glew SOURCE_DIR)
	set(GLEW_INCLUDE_DIR "${SOURCE_DIR}/include")
	set(GLEW_LIB "${SOURCE_DIR}/lib/Release/${GLEW_ARCHITECTURE}/glew32s.lib")
endif ()
