cmake_minimum_required(VERSION 2.8)

project(crag)
add_executable(crag 
	src/pch.cpp
	src/main.cpp
	src/applet/AppletBase.cpp
	src/applet/Engine.cpp
	src/applet/TestScript.cpp
	src/applet/ObserverScript.cpp
	src/ga/main.cpp
	src/gfx/Program.cpp
	src/gfx/Shader.cpp
	src/gfx/object/Light.cpp
	src/gfx/object/FormationMesh.cpp
	src/gfx/object/Ball.cpp
	src/gfx/object/Thruster.cpp
	src/gfx/object/Object.cpp
	src/gfx/object/Box.cpp
	src/gfx/object/Skybox.cpp
	src/gfx/object/Planet.cpp
	src/gfx/object/BranchNode.cpp
	src/gfx/object/Puff.cpp
	src/gfx/object/LeafNode.cpp
	src/gfx/Texture.cpp
	src/gfx/Scene.cpp
	src/gfx/RenderBuffer.cpp
	src/gfx/Layer.cpp
	src/gfx/Cuboid.cpp
	src/gfx/Fence.cpp
	src/gfx/FrameBuffer.cpp
	src/gfx/IndexBuffer.cpp
	src/gfx/Engine.cpp
	src/gfx/glHelpers.cpp
	src/gfx/ResourceManager.cpp
	src/gfx/Image.cpp
	src/gfx/Debug.cpp
	src/gfx/Pov.cpp
	src/gfx/Font.cpp
	src/gfx/Quad.cpp
	src/physics/IntersectionFunctor.cpp
	src/physics/Engine.cpp
	src/physics/Body.cpp
	src/physics/BoxBody.cpp
	src/physics/SphericalBody.cpp
	src/smp/Fiber.cpp
	src/smp/Uid.cpp
	src/smp/Semaphore.cpp
	src/smp/Thread.cpp
	src/smp/ReadersWriterMutex.cpp
	src/smp/scheduler.cpp
	src/smp/smp.cpp
	src/sim/PlanetShader.cpp
	src/sim/EntitySet.cpp
	src/sim/Firmament.cpp
	src/sim/Entity.cpp
	src/sim/Observer.cpp
	src/sim/PlanetaryBody.cpp
	src/sim/UserInput.cpp
	src/sim/FrequencyEstimator.cpp
	src/sim/Engine.cpp
	src/sim/Ball.cpp
	src/sim/EntityFunctions.cpp
	src/sim/Vehicle.cpp
	src/sim/Box.cpp
	src/sim/gravity.cpp
	src/sim/Planet.cpp
	src/sim/Star.cpp
	src/sim/MoonShader.cpp
	src/sim/Controller.cpp
	src/core/profile.cpp
	src/core/ConfigManager.cpp
	src/core/memory.cpp
	src/core/app.cpp
	src/core/ConfigEntry.cpp
	src/core/Statistics.cpp
	src/core/Random.cpp
	src/core/EventWatcher.cpp
	src/core/debug.cpp
	src/form/scene/VertexBuffer.cpp
	src/form/scene/RegulatorScript.cpp
	src/form/scene/Scene.cpp
	src/form/scene/Polyhedron.cpp
	src/form/scene/MeshBufferObject.cpp
	src/form/scene/Vertex.cpp
	src/form/scene/Mesh.cpp
	src/form/node/Point.cpp
	src/form/node/NodeBuffer.cpp
	src/form/node/Node.cpp
	src/form/node/CalculateNodeScoreFunctor.cpp
	src/form/node/PointBuffer.cpp
	src/form/node/RootNode.cpp
	src/form/Engine.cpp
	src/form/Formation.cpp)

include_directories("src" "dependencies/include")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-4.7")
set(CMAKE_CXX_FLAGS "-std=gnu++11 -g -pthread -DdDOUBLE -DPROFILE -Wall -Wfatal-errors")
#set(CMAKE_CXX_FLAGS "-std=gnu++11 -O1 -pthread -DdDOUBLE -DNDEBUG -Wall -Wfatal-errors")

find_package(OpenGL REQUIRED)
find_library(SDL2_LIBRARY SDL2 dependencies/lib)
target_link_libraries(crag ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} "ode" "GLEW" "pthread")

