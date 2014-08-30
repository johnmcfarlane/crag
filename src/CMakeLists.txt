cmake_minimum_required(VERSION 2.8)

add_executable(crag 
	applet/Applet.cpp
	applet/Engine.cpp
	core/app.cpp
	core/ConfigEntry.cpp
	core/ConfigManager.cpp
	core/debug.cpp
	core/EventWatcher.cpp
	core/GlobalResourceManager.cpp
	core/memory.cpp
	core/profile.cpp
	core/Random.cpp
	core/Resource.cpp
	core/ResourceManager.cpp
	core/Roster.cpp
	core/Statistics.cpp
	core/TypeId.cpp
	entity/gfx/Planet.cpp
	entity/gfx/Puff.cpp
	entity/gfx/Thruster.cpp
	entity/physics/PlanetBody.cpp
	entity/sim/AnimatController.cpp
	entity/sim/AnimatThruster.cpp
	entity/sim/CameraController.cpp
	entity/sim/Contact.cpp
	entity/sim/Genome.cpp
	entity/sim/HoverThruster.cpp
	entity/sim/MoonShader.cpp
	entity/sim/MouseObserverController.cpp
	entity/sim/ObserverInput.cpp
	entity/sim/PlanetController.cpp
	entity/sim/PlanetShader.cpp
	entity/sim/RoverThruster.cpp
	entity/sim/Sensor.cpp
	entity/sim/Thruster.cpp
	entity/sim/TouchObserverController.cpp
	entity/sim/UfoController1.cpp
	entity/sim/UfoController2.cpp
	entity/sim/VehicleController.cpp
	entity/sim/VernierThruster.cpp
	entity/SpawnEntityFunctions.cpp
	entity/SpawnSkybox.cpp
	entity/SpawnPlayer.cpp
	form/CalculateNodeScoreFunctor.cpp
	form/CastRay.cpp
	form/Engine.cpp
	form/Formation.cpp
	form/Mesh.cpp
	form/NodeBuffer.cpp
	form/Node.cpp
	form/Object.cpp
	form/Point.cpp
	form/PointBuffer.cpp
	form/Polyhedron.cpp
	form/QuaternaBuffer.cpp
	form/RayCastResult.cpp
	form/Scene.cpp
	form/Surrounding.cpp
	gfx/Debug.cpp
	gfx/Engine.cpp
	gfx/Font.cpp
	gfx/FrameBuffer.cpp
	gfx/glHelpers.cpp
	gfx/Image.cpp
	gfx/object/Ball.cpp
	gfx/object/Surrounding.cpp
	gfx/object/Light.cpp
	gfx/object/Object.cpp
	gfx/object/SearchLight.cpp
	gfx/object/Skybox.cpp
	gfx/object/MeshObject.cpp
	gfx/Frustum.cpp
	gfx/PlainVertex.cpp
	gfx/Pov.cpp
	gfx/Program.cpp
	gfx/Quad.cpp
	gfx/RegisterResources.cpp
	gfx/RenderBuffer.cpp
	gfx/Scene.cpp
	gfx/Shader.cpp
	gfx/Texture.cpp
	gfx/LitVertex.cpp
	gfx/Uniform.cpp
	main.cpp
	pch.cpp
	physics/Body.cpp
	physics/BoxBody.cpp
	physics/CylinderBody.cpp
	physics/Engine.cpp
	physics/GhostBody.cpp
	physics/Location.cpp
	physics/MeshBody.cpp
	physics/PassiveLocation.cpp
	physics/RayCast.cpp
	physics/SphereBody.cpp
	scripts/GameScript.cpp
	scripts/InitSpace.cpp
	scripts/MainScript.cpp
	scripts/MonitorOrigin.cpp
	scripts/RegulatorScript.cpp
	scripts/TestScript.cpp
	sim/Controller.cpp
	sim/Engine.cpp
	sim/Entity.cpp
	sim/EntityFunctions.cpp
	sim/gravity.cpp
	ipc/ListenerInterface.cpp
	ipc/Fiber.cpp
	ipc/FiberAndroid.cpp
	ipc/FiberPosix.cpp
	ipc/FiberWin.cpp
	smp/ReadersWriterMutex.cpp
	smp/Semaphore.cpp
	smp/smp.cpp
	smp/Thread.cpp
	ipc/Uid.cpp)

if (CRAG_RPI)
	include_directories("." "../rpi/include" "../rpi/include/SDL2" "/opt/vc/include")
	find_library(GLES2_LIBRARY GLESv2 /opt/vc/lib)
	target_link_libraries(crag ${SDL2_LIBRARY} ${ODE_LIBRARY} ${GLES2_LIBRARY} "dl" "pthread" "stdc++" "m")
else (CRAG_RPI)
	include_directories("." "/usr/include/SDL2" "../linux/include/SDL2")

	# TODO: Given these don't appear to be needed by RPi, try removing them here
	find_package(OpenGL REQUIRED)
	find_library(SDL2_LIBRARY SDL2 /usr/lib ../linux/lib)
	find_library(SDL2_IMAGE_LIBRARY SDL2_image /usr/lib ../linux/lib)
	find_library(ODE_LIBRARY ode /usr/lib)
	target_link_libraries(crag ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${ODE_LIBRARY} ${OPENGL_LIBRARIES} "dl" "pthread" "stdc++" "m")
endif (CRAG_RPI)

include (PrecompiledHeader.cmake)
add_precompiled_header(crag pch.h)
