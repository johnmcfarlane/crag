
include(ExternalProject)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(SDL2_IMAGE_URL "https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.1-VC.zip")
	set(SDL2_IMAGE_URL_MD5 "00338534c605262dcbf8ffe90ec2785d")

	ExternalProject_Add(
			sdl2_image
			URL "${SDL2_IMAGE_URL}"
			URL_MD5 "${SDL2_IMAGE_URL_MD5}"
			CONFIGURE_COMMAND ""
			BUILD_COMMAND ""
			INSTALL_COMMAND ""
	)

	if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
		set(SDL2_IMAGE_ARCHITECTURE "x64")
	elseif (${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015")
		set(SDL2_IMAGE_ARCHITECTURE "x86")
	else ()
		message(FATAL_ERROR "Unrecognized MSVC generator, ${CMAKE_GENERATOR}")
	endif ()
	
	ExternalProject_Get_Property(sdl2_image SOURCE_DIR)
	set(SDL2_IMAGE_INCLUDE_DIR "${SOURCE_DIR}/include")
	set(SDL2_IMAGE_LIBRARY "${SOURCE_DIR}/lib/${SDL2_IMAGE_ARCHITECTURE}/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_image${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(SDL2_IMAGE_DLL "${SOURCE_DIR}/lib/${SDL2_IMAGE_ARCHITECTURE}/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2_image${CMAKE_SHARED_LIBRARY_SUFFIX}")
else ()
	set(SDL2_IMAGE_URL "https://hg.libsdl.org/SDL_image/archive/20954174d833.tar.bz2")
	set(SDL2_IMAGE_URL_MD5 "fa5e1a63b13eb4f9808f42718f3af89f")

	ExternalProject_Add(
			sdl2_image
			URL "${SDL2_IMAGE_URL}"
			URL_MD5 "${SDL2_IMAGE_URL_MD5}"
			UPDATE_COMMAND ""
			CONFIGURE_COMMAND cd <SOURCE_DIR> && ./autogen.sh COMMAND export "SDL2_CONFIG=${SDL2_CONFIG_SCRIPT}" && <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
			BUILD_COMMAND $(MAKE)
			INSTALL_COMMAND $(MAKE) install
			TEST_COMMAND ""
	)

	ExternalProject_Get_Property(sdl2_image INSTALL_DIR)
	set(SDL2_IMAGE_INCLUDE_DIR "${INSTALL_DIR}/include/SDL2")
	set(SDL2_IMAGE_LIBRARY "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SDL2_image${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()

add_dependencies(sdl2_image sdl2)
