cmake_minimum_required(VERSION 2.8)

add_executable(crag 
	applet/Applet.cpp
	applet/Engine.cpp
	core/app.cpp
	core/ConfigEntry.cpp
	core/ConfigManager.cpp
	core/debug.cpp
	core/EventWatcher.cpp
	core/memory.cpp
	core/profile.cpp
	core/Random.cpp
	core/Roster.cpp
	core/Statistics.cpp
	entity/animat/AnimatController.cpp
	entity/animat/AnimatThruster.cpp
	entity/animat/Genome.cpp
	entity/animat/Sensor.cpp
	entity/observer/Contact.cpp
	entity/observer/MouseObserverController.cpp
	entity/observer/ObserverInput.cpp
	entity/observer/TouchObserverController.cpp
	entity/planet/gfx/Planet.cpp
	entity/planet/physics/PlanetBody.cpp
	entity/planet/sim/MoonShader.cpp
	entity/planet/sim/PlanetController.cpp
	entity/planet/sim/PlanetShader.cpp
	entity/SpawnEntityFunctions.cpp
	entity/SpawnSkybox.cpp
	entity/vehicle/gfx/Puff.cpp
	entity/vehicle/gfx/Thruster.cpp
	entity/vehicle/sim/RoverThruster.cpp
	entity/vehicle/sim/Thruster.cpp
	entity/vehicle/sim/VehicleController.cpp
	form/CalculateNodeScoreFunctor.cpp
	form/CastRay.cpp
	form/Engine.cpp
	form/Formation.cpp
	form/MeshBufferObject.cpp
	form/Mesh.cpp
	form/NodeBuffer.cpp
	form/Node.cpp
	form/Point.cpp
	form/PointBuffer.cpp
	form/Polyhedron.cpp
	form/RayCastResult.cpp
	form/RootNode.cpp
	form/Scene.cpp
	form/VertexBuffer.cpp
	form/Vertex.cpp
	gfx/Cuboid.cpp
	gfx/Debug.cpp
	gfx/Engine.cpp
	gfx/Fence.cpp
	gfx/Font.cpp
	gfx/FrameBuffer.cpp
	gfx/glHelpers.cpp
	gfx/Image.cpp
	gfx/IndexBuffer.cpp
	gfx/object/Ball.cpp
	gfx/object/Box.cpp
	gfx/object/FormationMesh.cpp
	gfx/object/LeafNode.cpp
	gfx/object/Light.cpp
	gfx/object/Object.cpp
	gfx/object/RootNode.cpp
	gfx/object/Skybox.cpp
	gfx/Pov.cpp
	gfx/Program.cpp
	gfx/Quad.cpp
	gfx/RenderBuffer.cpp
	gfx/ResourceManager.cpp
	gfx/Scene.cpp
	gfx/Shader.cpp
	gfx/Texture.cpp
	main.cpp
	pch.cpp
	physics/Body.cpp
	physics/BoxBody.cpp
	physics/Engine.cpp
	physics/Location.cpp
	physics/MeshSurround.cpp
	physics/PassiveLocation.cpp
	physics/RayCast.cpp
	physics/SphericalBody.cpp
	scripts/MainScript.cpp
	scripts/MonitorOrigin.cpp
	scripts/RegulatorScript.cpp
	scripts/TestScript.cpp
	sim/Controller.cpp
	sim/Engine.cpp
	sim/Entity.cpp
	sim/EntityFunctions.cpp
	sim/FrequencyEstimator.cpp
	sim/gravity.cpp
	ipc/ListenerInterface.cpp
	ipc/Fiber.cpp
	ipc/FiberAndroid.cpp
	ipc/FiberPosix.cpp
	ipc/FiberWin.cpp
	smp/ReadersWriterMutex.cpp
	smp/scheduler.cpp
	smp/Semaphore.cpp
	smp/smp.cpp
	smp/Thread.cpp
	ipc/Uid.cpp)

include_directories("." "../dependencies/include")

find_package(OpenGL REQUIRED)
find_library(SDL2_LIBRARY SDL2 ../dependencies/lib)
find_library(ODE_LIBRARY ode ../dependencies/lib)
target_link_libraries(crag ${SDL2_LIBRARY} ${ODE_LIBRARY} ${OPENGL_LIBRARIES} "dl" "GLEW" "pthread" "stdc++" "m")

include (PrecompiledHeader.cmake)
add_precompiled_header(crag pch.h)
