cmake_minimum_required(VERSION 2.8)

project(crag)
add_executable(crag 
	src/applet/Applet.cpp
	src/applet/Engine.cpp
	src/core/app.cpp
	src/core/ConfigEntry.cpp
	src/core/ConfigManager.cpp
	src/core/debug.cpp
	src/core/EventWatcher.cpp
	src/core/memory.cpp
	src/core/profile.cpp
	src/core/Random.cpp
	src/core/Roster.cpp
	src/core/Statistics.cpp
	src/form/Engine.cpp
	src/form/Formation.cpp
	src/form/node/CalculateNodeScoreFunctor.cpp
	src/form/node/NodeBuffer.cpp
	src/form/node/Node.cpp
	src/form/node/PointBuffer.cpp
	src/form/node/Point.cpp
	src/form/node/RootNode.cpp
	src/form/scene/MeshBufferObject.cpp
	src/form/scene/Mesh.cpp
	src/form/scene/Polyhedron.cpp
	src/form/scene/Scene.cpp
	src/form/scene/VertexBuffer.cpp
	src/form/scene/Vertex.cpp
	src/gfx/Cuboid.cpp
	src/gfx/Debug.cpp
	src/gfx/Engine.cpp
	src/gfx/Fence.cpp
	src/gfx/Font.cpp
	src/gfx/FrameBuffer.cpp
	src/gfx/glHelpers.cpp
	src/gfx/Image.cpp
	src/gfx/IndexBuffer.cpp
	src/gfx/Layer.cpp
	src/gfx/object/Ball.cpp
	src/gfx/object/Box.cpp
	src/gfx/object/FormationMesh.cpp
	src/gfx/object/LeafNode.cpp
	src/gfx/object/Light.cpp
	src/gfx/object/Object.cpp
	src/gfx/object/RootNode.cpp
	src/gfx/object/Skybox.cpp
	src/gfx/Pov.cpp
	src/gfx/Program.cpp
	src/gfx/Quad.cpp
	src/gfx/RenderBuffer.cpp
	src/gfx/ResourceManager.cpp
	src/gfx/Scene.cpp
	src/gfx/Shader.cpp
	src/gfx/Texture.cpp
	src/main.cpp
	src/pch.cpp
	src/physics/Body.cpp
	src/physics/BoxBody.cpp
	src/physics/Engine.cpp
	src/physics/FixedLocation.cpp
	src/physics/IntersectionFunctor.cpp
	src/physics/Location.cpp
	src/physics/SphericalBody.cpp
	src/scripts/MainScript.cpp
	src/scripts/MonitorOrigin.cpp
	src/scripts/planet/gfx/Planet.cpp
	src/scripts/planet/physics/PlanetBody.cpp
	src/scripts/planet/sim/MoonShader.cpp
	src/scripts/planet/sim/PlanetController.cpp
	src/scripts/planet/sim/PlanetShader.cpp
	src/scripts/RegulatorScript.cpp
	src/scripts/SpawnEntityFunctions.cpp
	src/scripts/TestScript.cpp
	src/scripts/vehicle/gfx/Puff.cpp
	src/scripts/vehicle/gfx/Thruster.cpp
	src/scripts/vehicle/sim/RoverThruster.cpp
	src/scripts/vehicle/sim/Thruster.cpp
	src/scripts/vehicle/sim/VehicleController.cpp
	src/sim/Controller.cpp
	src/sim/Engine.cpp
	src/sim/Entity.cpp
	src/sim/EntityFunctions.cpp
	src/sim/FrequencyEstimator.cpp
	src/sim/gravity.cpp
	src/sim/ObserverController.cpp
	src/sim/ObserverInput.cpp
	src/ipc/Fiber.cpp
	src/ipc/FiberAndroid.cpp
	src/ipc/FiberPosix.cpp
	src/ipc/FiberWin.cpp
	src/smp/ReadersWriterMutex.cpp
	src/smp/scheduler.cpp
	src/smp/Semaphore.cpp
	src/smp/smp.cpp
	src/smp/Thread.cpp
	src/ipc/Uid.cpp)

include_directories("src" "dependencies/include")
#set(CMAKE_CXX_COMPILER "/usr/bin/colorgcc")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# TODO: -Weffc++ -fno-unwind-tables
set(CMAKE_CXX_FLAGS "-std=gnu++11 -g -pthread -DPROFILE -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -fno-rtti -fno-exceptions")
#set(CMAKE_CXX_FLAGS "-std=gnu++11 -O1 -pthread -D__STRICT_ANSI__ -DNDEBUG -Wall -Wextra -Wfatal-errors -fno-rtti -fno-exceptions")

find_package(OpenGL REQUIRED)
find_library(SDL2_LIBRARY SDL2 dependencies/lib)
target_link_libraries(crag ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} "ode" "dl" "GLEW" "pthread")

include (PrecompiledHeader.cmake)

# with: 24.336s
# without: 63.250s
add_precompiled_header(crag src/pch.h)

