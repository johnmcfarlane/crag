
include(ExternalProject)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(SDL2_URL "https://www.libsdl.org/release/SDL2-devel-2.0.5-VC.zip")
	set(SDL2_URL_MD5 "b771f36bb640c173e53fa8259cdd3835")

	ExternalProject_Add(
			sdl2
			URL "${SDL2_URL}"
			URL_MD5 "${SDL2_URL_MD5}"
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ""
			INSTALL_COMMAND ""
	)

	if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
		set(SDL2_ARCHITECTURE "x64")
	elseif (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015")
		set(SDL2_ARCHITECTURE "x86")
	else ()
		message(FATAL_ERROR "Unrecognized MSVC generator, ${CMAKE_GENERATOR}")
	endif ()
	
	ExternalProject_Get_Property(sdl2 SOURCE_DIR)
	set(SDL2_INCLUDE_DIR "${SOURCE_DIR}/include")
	set(SDL2_LIBRARY "${SOURCE_DIR}/lib/${SDL2_ARCHITECTURE}/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(SDL2_MAIN_LIBRARY "${SOURCE_DIR}/lib/${SDL2_ARCHITECTURE}/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2main${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(SDL2_DLL "${SOURCE_DIR}/lib/${SDL2_ARCHITECTURE}/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(SDL2_README "${SOURCE_DIR}/README-SDL.txt")
else ()
	set(SDL2_URL "https://hg.libsdl.org/SDL/archive/007dfe83abf8.tar.bz2")
	set(SDL2_URL_MD5 "9df707d8aa0d0a3dc14d7fcacc77da71")
		
	ExternalProject_Add(
			sdl2
			URL "${SDL2_URL}"
			URL_MD5 "${SDL2_URL_MD5}"
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
	)
	ExternalProject_Get_Property(sdl2 INSTALL_DIR)
	set(SDL2_INCLUDE_DIR "${INSTALL_DIR}/include/SDL2")
	set(SDL2_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(SDL2_CONFIG_SCRIPT "${INSTALL_DIR}/bin/sdl2-config")
endif ()