cmake_minimum_required(VERSION 2.8.12)

if (POLICY CMP0059)
cmake_policy(SET CMP0059 NEW)
endif ()
if (POLICY CMP0046)
cmake_policy(SET CMP0046 NEW)
endif ()

project(crag)

# dependencies
include("cmake/common.cmake")
include("cmake/fixed_point.cmake")
include("cmake/GLEW.cmake")
include("cmake/ODE.cmake")
include("cmake/SDL2.cmake")
include("cmake/SDL2_image.cmake")

# crag binary target
add_subdirectory("src")

# installation instructions
install(
	FILES ${CRAG_ARCHIVE_FILES}
	DESTINATION "."
)
install(
	DIRECTORY assets
	DESTINATION "."
)
install(
	PROGRAMS "${CRAG_EXECUTABLE_FILE}"
	DESTINATION "."
)

# Organize VS Solution
source_group_by_folder("${PROJECT_FILES}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT crag)
