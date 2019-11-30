
include(ExternalProject)

set(ODE_URL "https://bitbucket.org/odedevs/ode/downloads/ode-0.14.tar.gz")
set(ODE_URL_MD5 "c5e154d45656a8a70092a4a7c5214fc0")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
		set(ODE_PREMAKE_PLATFORM "x64")
		set(ODE_MSBUILD_PLATFORM "x64")
	elseif (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015")
		set(ODE_PREMAKE_PLATFORM "x32")
		set(ODE_MSBUILD_PLATFORM "Win32")
	else ()
		message(FATAL_ERROR "Unrecognized MSVC generator, ${CMAKE_GENERATOR}")
	endif ()
	
	ExternalProject_Add(
			opende
			URL "${ODE_URL}"
			URL_MD5 "${ODE_URL_MD5}"
			UPDATE_COMMAND ""
			CONFIGURE_COMMAND cd <SOURCE_DIR>\\build && premake4 --platform=${ODE_PREMAKE_PLATFORM} --only-single --only-static vs2010
				COMMAND "devenv.exe" -Upgrade <SOURCE_DIR>\\build\\vs2010\\ode.sln
			BUILD_COMMAND MSBuild <SOURCE_DIR>\\build\\vs2010\\ode.sln /property:Configuration=Release /property:Platform=${ODE_MSBUILD_PLATFORM}
				COMMAND MSBuild <SOURCE_DIR>\\build\\vs2010\\ode.sln /property:Configuration=Debug /property:Platform=${ODE_MSBUILD_PLATFORM}
			INSTALL_COMMAND ""
	)

	ExternalProject_Get_Property(opende SOURCE_DIR)
	set(ODE_INCLUDE_DIR "${SOURCE_DIR}/include")
	set(ODE_LIBRARY_OPTIMIZED "${SOURCE_DIR}/lib/Release/${CMAKE_STATIC_LIBRARY_PREFIX}ode${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(ODE_LIBRARY_DEBUG "${SOURCE_DIR}/lib/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}oded${CMAKE_STATIC_LIBRARY_SUFFIX}")
else ()
	set(ODE_REPOSITORY "https://bitbucket.org/odedevs/ode")
	set(ODE_TAG "b137611c5d7d")

	ExternalProject_Add(
			opende
			URL "${ODE_URL}"
			URL_MD5 "${ODE_URL_MD5}"
			UPDATE_COMMAND ""
			CONFIGURE_COMMAND cd <SOURCE_DIR> && ./bootstrap COMMAND <SOURCE_DIR>/configure --disable-demos --with-trimesh=opcode --prefix=<INSTALL_DIR>
			BUILD_COMMAND $(MAKE) COMMAND $(MAKE) install
			INSTALL_COMMAND ""
	)

	ExternalProject_Get_Property(opende INSTALL_DIR)
	set(ODE_INCLUDE_DIR "${INSTALL_DIR}/include")
	set(ODE_LIBRARY_OPTIMIZED "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ode${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(ODE_LIBRARY_DEBUG "${ODE_LIBRARY_OPTIMIZED}")
endif ()
