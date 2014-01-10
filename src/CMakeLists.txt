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
	form/Mesh.cpp
	form/NodeBuffer.cpp
	form/Node.cpp
	form/Point.cpp
	form/PointBuffer.cpp
	form/Polyhedron.cpp
	form/QuaternaBuffer.cpp
	form/RayCastResult.cpp
	form/Scene.cpp
	form/Surrounding.cpp
	gfx/Debug.cpp
	gfx/Engine.cpp
	gfx/Fence.cpp
	gfx/Font.cpp
	gfx/FrameBuffer.cpp
	gfx/glHelpers.cpp
	gfx/Image.cpp
	gfx/object/Ball.cpp
	gfx/object/Box.cpp
	gfx/object/FormationGeneration.cpp
	gfx/object/Surrounding.cpp
	gfx/object/LeafNode.cpp
	gfx/object/Light.cpp
	gfx/object/Object.cpp
	gfx/object/RootNode.cpp
	gfx/object/Skybox.cpp
	gfx/PlainVertex.cpp
	gfx/Pov.cpp
	gfx/Program.cpp
	gfx/Quad.cpp
	gfx/RenderBuffer.cpp
	gfx/ResourceManager.cpp
	gfx/Scene.cpp
	gfx/Shader.cpp
	gfx/Texture.cpp
	gfx/LitVertex.cpp
	main.cpp
	pch.cpp
	physics/Body.cpp
	physics/BoxBody.cpp
	physics/Engine.cpp
	physics/GhostBody.cpp
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

if (CRAG_RPI)
	include_directories("." "../rpi/include" "../rpi/include/SDL2" "/opt/vc/include")
	find_library(GLES2_LIBRARY GLESv2 /opt/vc/lib)
	target_link_libraries(crag ${SDL2_LIBRARY} ${ODE_LIBRARY} ${GLES2_LIBRARY} "dl" "pthread" "stdc++" "m")
else (CRAG_RPI)
	include_directories("." "/usr/include/SDL2")

	# TODO: Given these don't appear to be needed by RPi, try removing them here
	find_package(OpenGL REQUIRED)
	find_library(SDL2_LIBRARY SDL2 /usr/lib)
	find_library(ODE_LIBRARY ode /usr/lib)
	target_link_libraries(crag ${SDL2_LIBRARY} ${ODE_LIBRARY} ${OPENGL_LIBRARIES} "dl" "GLEW" "pthread" "stdc++" "m")
endif (CRAG_RPI)

include (PrecompiledHeader.cmake)
add_precompiled_header(crag pch.h)
