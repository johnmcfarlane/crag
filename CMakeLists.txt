cmake_minimum_required(VERSION 2.8.12)

if (POLICY CMP0059)
cmake_policy(SET CMP0059 NEW)
endif ()
if (POLICY CMP0046)
cmake_policy(SET CMP0046 NEW)
endif ()

project(crag)

include("cmake/fixed_point.cmake")
include("cmake/GLEW.cmake")
include("cmake/ODE.cmake")
include("cmake/SDL2.cmake")
include("cmake/SDL2_image.cmake")

######################################################################
# crag executable

set(CRAG_SOURCE_DIRECTORY "src")

set(SOURCE_FILES
    ${CRAG_SOURCE_DIRECTORY}/applet/Applet.cpp
    ${CRAG_SOURCE_DIRECTORY}/applet/Applet.h
    ${CRAG_SOURCE_DIRECTORY}/applet/AppletInterface.h
    ${CRAG_SOURCE_DIRECTORY}/applet/AppletInterface_Impl.h
    ${CRAG_SOURCE_DIRECTORY}/applet/Engine.cpp
    ${CRAG_SOURCE_DIRECTORY}/applet/Engine.h
    ${CRAG_SOURCE_DIRECTORY}/core/app.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/app.h
    ${CRAG_SOURCE_DIRECTORY}/core/config.h
    ${CRAG_SOURCE_DIRECTORY}/core/ConfigEntry.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/ConfigEntry.h
    ${CRAG_SOURCE_DIRECTORY}/core/ConfigInit.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/core.h
    ${CRAG_SOURCE_DIRECTORY}/core/counted_object.h
    ${CRAG_SOURCE_DIRECTORY}/core/debug.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/debug.h
    ${CRAG_SOURCE_DIRECTORY}/core/Enumeration.h
    ${CRAG_SOURCE_DIRECTORY}/core/EventWatcher.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/EventWatcher.h
    ${CRAG_SOURCE_DIRECTORY}/core/fixed_point.h
    ${CRAG_SOURCE_DIRECTORY}/core/floatOps.h
    ${CRAG_SOURCE_DIRECTORY}/core/function_ref.h
    ${CRAG_SOURCE_DIRECTORY}/core/GlobalResourceManager.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/GlobalResourceManager.h
    ${CRAG_SOURCE_DIRECTORY}/core/HashString.h
    ${CRAG_SOURCE_DIRECTORY}/core/intOps.h
    ${CRAG_SOURCE_DIRECTORY}/core/intrusive_list.h
    ${CRAG_SOURCE_DIRECTORY}/core/memory.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/memory.h
    ${CRAG_SOURCE_DIRECTORY}/core/object_pool.h
    ${CRAG_SOURCE_DIRECTORY}/core/pointer_union.h
    ${CRAG_SOURCE_DIRECTORY}/core/profile.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/profile.h
    ${CRAG_SOURCE_DIRECTORY}/core/Random.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/Random.h
    ${CRAG_SOURCE_DIRECTORY}/core/Resource.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/Resource.h
    ${CRAG_SOURCE_DIRECTORY}/core/ResourceHandle.h
    ${CRAG_SOURCE_DIRECTORY}/core/ResourceManager.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/ResourceManager.h
    ${CRAG_SOURCE_DIRECTORY}/core/ring_buffer.h
    ${CRAG_SOURCE_DIRECTORY}/core/Roster.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/Roster.h
    ${CRAG_SOURCE_DIRECTORY}/core/RosterObjectDeclare.h
    ${CRAG_SOURCE_DIRECTORY}/core/RosterObjectDefine.h
    ${CRAG_SOURCE_DIRECTORY}/core/Sigmoid.h
    ${CRAG_SOURCE_DIRECTORY}/core/Statistics.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/Statistics.h
    ${CRAG_SOURCE_DIRECTORY}/core/TypeId.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/TypeId.h
    ${CRAG_SOURCE_DIRECTORY}/core/unit_interval.h
    ${CRAG_SOURCE_DIRECTORY}/core/unit_interval.cpp
    ${CRAG_SOURCE_DIRECTORY}/core/verify.h
    ${CRAG_SOURCE_DIRECTORY}/core/windows.h
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Planet.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Planet.h
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Puff.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Puff.h
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Thruster.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/gfx/Thruster.h
    ${CRAG_SOURCE_DIRECTORY}/entity/physics/AnimatBody.h
    ${CRAG_SOURCE_DIRECTORY}/entity/physics/AnimatBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/physics/PlanetBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/physics/PlanetBody.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/AnimatController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/AnimatController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/AnimatModel.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/AnimatModel.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/CameraController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/CameraController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Contact.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Contact.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Gene.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Gene.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Genome.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Genome.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Health.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Health.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/KeyboardTransmitter.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/KeyboardTransmitter.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/MoonShader.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/MoonShader.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/MouseObserverController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/MouseObserverController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/nnet.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/nnet.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/ObserverInput.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/ObserverInput.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/PlanetController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/PlanetController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/PlanetShader.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/PlanetShader.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Sensor.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Sensor.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Signal.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Signal.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/ssga.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/ssga.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Thruster.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/Thruster.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/TouchObserverController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/TouchObserverController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/UfoController1.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/UfoController1.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/UfoController2.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/UfoController2.h
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/VehicleController.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/sim/VehicleController.h
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnEntityFunctions.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnEntityFunctions.h
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnPlayer.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnPlayer.h
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnSkybox.cpp
    ${CRAG_SOURCE_DIRECTORY}/entity/SpawnSkybox.h
    ${CRAG_SOURCE_DIRECTORY}/form/CalculateNodeScoreFunctor.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/CalculateNodeScoreFunctor.h
    ${CRAG_SOURCE_DIRECTORY}/form/CastRay.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/CastRay.h
    ${CRAG_SOURCE_DIRECTORY}/form/defs.h
    ${CRAG_SOURCE_DIRECTORY}/form/Engine.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Engine.h
    ${CRAG_SOURCE_DIRECTORY}/form/ForEachChildNode.h
    ${CRAG_SOURCE_DIRECTORY}/form/ForEachFaceInSphere.h
    ${CRAG_SOURCE_DIRECTORY}/form/ForEachNodeFace.h
    ${CRAG_SOURCE_DIRECTORY}/form/Formation.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Formation.h
    ${CRAG_SOURCE_DIRECTORY}/form/GatherExpandableNodesFunctor.h
    ${CRAG_SOURCE_DIRECTORY}/form/Mesh.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Mesh.h
    ${CRAG_SOURCE_DIRECTORY}/form/MeshProperties.h
    ${CRAG_SOURCE_DIRECTORY}/form/Node.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Node.h
    ${CRAG_SOURCE_DIRECTORY}/form/NodeBuffer.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/NodeBuffer.h
    ${CRAG_SOURCE_DIRECTORY}/form/Object.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Object.h
    ${CRAG_SOURCE_DIRECTORY}/form/Point.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Point.h
    ${CRAG_SOURCE_DIRECTORY}/form/PointBuffer.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/PointBuffer.h
    ${CRAG_SOURCE_DIRECTORY}/form/Polyhedron.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Polyhedron.h
    ${CRAG_SOURCE_DIRECTORY}/form/Quaterna.h
    ${CRAG_SOURCE_DIRECTORY}/form/QuaternaBuffer.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/QuaternaBuffer.h
    ${CRAG_SOURCE_DIRECTORY}/form/RayCastResult.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/RayCastResult.h
    ${CRAG_SOURCE_DIRECTORY}/form/Scene.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Scene.h
    ${CRAG_SOURCE_DIRECTORY}/form/Shader.h
    ${CRAG_SOURCE_DIRECTORY}/form/Surrounding.cpp
    ${CRAG_SOURCE_DIRECTORY}/form/Surrounding.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Distance.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Intersection.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Magnitude.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Matrix.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Matrix33.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Matrix44.h
    ${CRAG_SOURCE_DIRECTORY}/geom/MatrixOps.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Plane.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Quaternion.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Ray.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Space.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Sphere.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Transformation.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Triangle.h
    ${CRAG_SOURCE_DIRECTORY}/geom/utils.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Vector.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Vector2.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Vector3.h
    ${CRAG_SOURCE_DIRECTORY}/geom/Vector4.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Ball.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Ball.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Light.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Light.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/MeshObject.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/MeshObject.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Object.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Object.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/SearchLight.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/SearchLight.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Skybox.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Skybox.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Surrounding.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/object/Surrounding.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/axes.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/BufferObject.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Color.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Debug.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Debug.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/defs.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Engine.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Engine.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Font.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Font.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Frustum.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Frustum.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/GenerateShadowVolumeMesh.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/glHelpers.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/glHelpers.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Image.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Image.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/IndexBufferObject.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/IndexedVboResource.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/LightType.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/LitVertex.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/LitVertex.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/LodParameters.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Mesh.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Messages.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/NonIndexedVboResource.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/PlainVertex.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/PlainVertex.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Pov.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Pov.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Program.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Program.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Quad.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Quad.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/RegisterResources.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/RegisterResources.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/RenderBuffer.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/RenderBuffer.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Scene.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Scene.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/SetCameraEvent.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/SetLodParametersEvent.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/SetSpaceEvent.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Shader.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Shader.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/ShadowMap.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/ShadowVolume.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Texture.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Texture2d.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Texture2d.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/TextureCubeMap.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/TextureCubeMap.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/Uniform.cpp
    ${CRAG_SOURCE_DIRECTORY}/gfx/Uniform.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/VboResource.h
    ${CRAG_SOURCE_DIRECTORY}/gfx/VertexBufferObject.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Daemon.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/EngineBase.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Fiber.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/Fiber.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberAndroid.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberAndroid.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberPosix.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberPosix.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberWin.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/FiberWin.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Future.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Handle.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Handle_Impl.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Listener.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/ListenerInterface.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/ListenerInterface.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/MessageQueue.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/MessageQueue_Impl.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/ObjectBase.h
    ${CRAG_SOURCE_DIRECTORY}/ipc/Uid.cpp
    ${CRAG_SOURCE_DIRECTORY}/ipc/Uid.h
    ${CRAG_SOURCE_DIRECTORY}/physics/Body.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/Body.h
    ${CRAG_SOURCE_DIRECTORY}/physics/BoxBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/BoxBody.h
    ${CRAG_SOURCE_DIRECTORY}/physics/CylinderBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/CylinderBody.h
    ${CRAG_SOURCE_DIRECTORY}/physics/defs.h
    ${CRAG_SOURCE_DIRECTORY}/physics/Engine.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/Engine.h
    ${CRAG_SOURCE_DIRECTORY}/physics/GhostBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/GhostBody.h
    ${CRAG_SOURCE_DIRECTORY}/physics/Location.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/Location.h
    ${CRAG_SOURCE_DIRECTORY}/physics/MeshBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/MeshBody.h
    ${CRAG_SOURCE_DIRECTORY}/physics/PassiveLocation.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/PassiveLocation.h
    ${CRAG_SOURCE_DIRECTORY}/physics/RayCast.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/RayCast.h
    ${CRAG_SOURCE_DIRECTORY}/physics/SphereBody.cpp
    ${CRAG_SOURCE_DIRECTORY}/physics/SphereBody.h
    ${CRAG_SOURCE_DIRECTORY}/scripts/GameScript.cpp
    ${CRAG_SOURCE_DIRECTORY}/scripts/GameScript.h
    ${CRAG_SOURCE_DIRECTORY}/scripts/InitSpace.cpp
    ${CRAG_SOURCE_DIRECTORY}/scripts/InitSpace.h
    ${CRAG_SOURCE_DIRECTORY}/scripts/MainScript.cpp
    ${CRAG_SOURCE_DIRECTORY}/scripts/MainScript.h
    ${CRAG_SOURCE_DIRECTORY}/scripts/MonitorOrigin.cpp
    ${CRAG_SOURCE_DIRECTORY}/scripts/MonitorOrigin.h
    ${CRAG_SOURCE_DIRECTORY}/scripts/RegulatorScript.cpp
    ${CRAG_SOURCE_DIRECTORY}/scripts/RegulatorScript.h
    ${CRAG_SOURCE_DIRECTORY}/sim/Controller.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/Controller.h
    ${CRAG_SOURCE_DIRECTORY}/sim/defs.h
    ${CRAG_SOURCE_DIRECTORY}/sim/Engine.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/Engine.h
    ${CRAG_SOURCE_DIRECTORY}/sim/Entity.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/Entity.h
    ${CRAG_SOURCE_DIRECTORY}/sim/EntityFunctions.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/EntityFunctions.h
    ${CRAG_SOURCE_DIRECTORY}/sim/gravity.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/gravity.h
    ${CRAG_SOURCE_DIRECTORY}/sim/Model.cpp
    ${CRAG_SOURCE_DIRECTORY}/sim/Model.h
    ${CRAG_SOURCE_DIRECTORY}/smp/ReadersWriterMutex.cpp
    ${CRAG_SOURCE_DIRECTORY}/smp/ReadersWriterMutex.h
    ${CRAG_SOURCE_DIRECTORY}/smp/Semaphore.cpp
    ${CRAG_SOURCE_DIRECTORY}/smp/Semaphore.h
    ${CRAG_SOURCE_DIRECTORY}/smp/SimpleMutex.h
    ${CRAG_SOURCE_DIRECTORY}/smp/smp.cpp
    ${CRAG_SOURCE_DIRECTORY}/smp/smp.h
    ${CRAG_SOURCE_DIRECTORY}/smp/Thread.cpp
    ${CRAG_SOURCE_DIRECTORY}/smp/Thread.h
    ${CRAG_SOURCE_DIRECTORY}/main.cpp
    ${CRAG_SOURCE_DIRECTORY}/pch.cpp
    ${CRAG_SOURCE_DIRECTORY}/pch.h)

add_executable(crag ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STRICT_ANSI__")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPROFILE")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -DPROFILE")

target_include_directories(crag PRIVATE ${CRAG_SOURCE_DIRECTORY})

add_dependencies(crag fixed_point opende sdl2 sdl2_image)
target_include_directories(crag SYSTEM PRIVATE "${FIXED_POINT_INCLUDE_DIR}" "${ODE_INCLUDE_DIR}" "${SDL2_INCLUDE_DIR}" "${SDL2_IMAGE_INCLUDE_DIR}")
target_link_libraries(crag "${SDL2_LIBRARY}" "${SDL2_IMAGE_LIBRARY}" "${SDL2_MAIN_LIBRARY}")
target_link_libraries(crag debug "${ODE_LIBRARY_DEBUG}")
target_link_libraries(crag optimized "${ODE_LIBRARY_OPTIMIZED}")

# Graphics library
find_package(OpenGL)
if (OPENGL_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRAG_GL")
	target_link_libraries(crag "${OPENGL_LIBRARIES}")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRAG_GLES")
	find_library(GLES2_LIBRARY GLESv2 "/opt/vc/lib")
	target_link_libraries(crag "${GLES2_LIBRARY}")
endif ()

# Raspberry Pi
find_library(BCM_HOST bcm_host /opt/vc/lib)
if (BCM_HOST)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCRAG_RPI")
	target_include_directories(crag SYSTEM PRIVATE "/opt/vc/include")
	target_link_libraries(crag "${BCM_HOST}" "rt")
endif (BCM_HOST)

# Windows or Linux
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /errorReport:prompt /nologo")  # /WX
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")                                 # /GL /LTCG

	add_dependencies(crag glew)
	target_include_directories(crag SYSTEM PRIVATE "${GLEW_INCLUDE_DIR}")
	target_link_libraries(crag "${GLEW_LIB}")

	set(CRAG_TARGET_FILE_DIR $<TARGET_FILE_DIR:crag>)

	add_custom_command(TARGET crag POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${SDL2_DLL}" "${CRAG_TARGET_FILE_DIR}")

	add_custom_command(TARGET crag POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${SDL2_README}" "${CRAG_TARGET_FILE_DIR}")

	add_custom_command(TARGET crag POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${SDL2_IMAGE_DLL}" "${CRAG_TARGET_FILE_DIR}")

	add_custom_command(TARGET crag POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CRAG_TARGET_FILE_DIR}")

	add_custom_command(TARGET crag POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_SOURCE_DIR}/assets ${CRAG_TARGET_FILE_DIR}/assets)

	add_custom_target(zip
			DEPENDS crag
			COMMAND
				cd "${CRAG_TARGET_FILE_DIR}"
			COMMAND
				${CMAKE_COMMAND} -E tar "cfv" "../crag.zip" --format=zip
				"${CRAG_TARGET_FILE_DIR}/SDL2.dll"
				"${CRAG_TARGET_FILE_DIR}/SDL2_image.dll"
				"${CRAG_TARGET_FILE_DIR}/README-SDL.txt"
				"${CRAG_TARGET_FILE_DIR}/crag.exe"
				"${CRAG_TARGET_FILE_DIR}/README.md"
				"${CRAG_TARGET_FILE_DIR}/assets")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-unwind-tables -Wall -Wextra -Wfatal-errors -Werror")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti -Ofast")
	set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
	set(CMAKE_EXE_LINKER_FLAGS "-g")
	target_link_libraries(crag "dl" "pthread" "stdc++" "m")
endif ()
