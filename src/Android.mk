LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

#SDL_PATH := ../SDL
SDL_PATH := ../project/android-crag/jni/SDL
CRAG_PATH := ../src

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include;.

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	$(SDL_PATH)/src/applet/Applet.cpp \
	$(SDL_PATH)/src/applet/Engine.cpp \
	$(SDL_PATH)/src/core/app.cpp \
	$(SDL_PATH)/src/core/ConfigEntry.cpp \
	$(SDL_PATH)/src/core/ConfigManager.cpp \
	$(SDL_PATH)/src/core/debug.cpp \
	$(SDL_PATH)/src/core/EventWatcher.cpp \
	$(SDL_PATH)/src/core/memory.cpp \
	$(SDL_PATH)/src/core/profile.cpp \
	$(SDL_PATH)/src/core/Random.cpp \
	$(SDL_PATH)/src/core/Roster.cpp \
	$(SDL_PATH)/src/core/Statistics.cpp \
	$(SDL_PATH)/src/form/Engine.cpp \
	$(SDL_PATH)/src/form/Formation.cpp \
	$(SDL_PATH)/src/form/node/CalculateNodeScoreFunctor.cpp \
	$(SDL_PATH)/src/form/node/NodeBuffer.cpp \
	$(SDL_PATH)/src/form/node/Node.cpp \
	$(SDL_PATH)/src/form/node/PointBuffer.cpp \
	$(SDL_PATH)/src/form/node/Point.cpp \
	$(SDL_PATH)/src/form/node/RootNode.cpp \
	$(SDL_PATH)/src/form/scene/MeshBufferObject.cpp \
	$(SDL_PATH)/src/form/scene/Mesh.cpp \
	$(SDL_PATH)/src/form/scene/Polyhedron.cpp \
	$(SDL_PATH)/src/form/scene/Scene.cpp \
	$(SDL_PATH)/src/form/scene/VertexBuffer.cpp \
	$(SDL_PATH)/src/form/scene/Vertex.cpp \
	$(SDL_PATH)/src/gfx/Cuboid.cpp \
	$(SDL_PATH)/src/gfx/Debug.cpp \
	$(SDL_PATH)/src/gfx/Engine.cpp \
	$(SDL_PATH)/src/gfx/Fence.cpp \
	$(SDL_PATH)/src/gfx/Font.cpp \
	$(SDL_PATH)/src/gfx/FrameBuffer.cpp \
	$(SDL_PATH)/src/gfx/glHelpers.cpp \
	$(SDL_PATH)/src/gfx/Image.cpp \
	$(SDL_PATH)/src/gfx/IndexBuffer.cpp \
	$(SDL_PATH)/src/gfx/Layer.cpp \
	$(SDL_PATH)/src/gfx/object/Ball.cpp \
	$(SDL_PATH)/src/gfx/object/Box.cpp \
	$(SDL_PATH)/src/gfx/object/FormationMesh.cpp \
	$(SDL_PATH)/src/gfx/object/LeafNode.cpp \
	$(SDL_PATH)/src/gfx/object/Light.cpp \
	$(SDL_PATH)/src/gfx/object/Object.cpp \
	$(SDL_PATH)/src/gfx/object/RootNode.cpp \
	$(SDL_PATH)/src/gfx/object/Skybox.cpp \
	$(SDL_PATH)/src/gfx/Pov.cpp \
	$(SDL_PATH)/src/gfx/Program.cpp \
	$(SDL_PATH)/src/gfx/Quad.cpp \
	$(SDL_PATH)/src/gfx/RenderBuffer.cpp \
	$(SDL_PATH)/src/gfx/ResourceManager.cpp \
	$(SDL_PATH)/src/gfx/Scene.cpp \
	$(SDL_PATH)/src/gfx/Shader.cpp \
	$(SDL_PATH)/src/gfx/Texture.cpp \
	$(SDL_PATH)/src/main.cpp \
	$(SDL_PATH)/src/pch.cpp \
	$(SDL_PATH)/src/physics/Body.cpp \
	$(SDL_PATH)/src/physics/BoxBody.cpp \
	$(SDL_PATH)/src/physics/Engine.cpp \
	$(SDL_PATH)/src/physics/FixedLocation.cpp \
	$(SDL_PATH)/src/physics/IntersectionFunctor.cpp \
	$(SDL_PATH)/src/physics/Location.cpp \
	$(SDL_PATH)/src/physics/SphericalBody.cpp \
	$(SDL_PATH)/src/scripts/MainScript.cpp \
	$(SDL_PATH)/src/scripts/MonitorOrigin.cpp \
	$(SDL_PATH)/src/scripts/planet/gfx/Planet.cpp \
	$(SDL_PATH)/src/scripts/planet/physics/PlanetBody.cpp \
	$(SDL_PATH)/src/scripts/planet/sim/MoonShader.cpp \
	$(SDL_PATH)/src/scripts/planet/sim/PlanetController.cpp \
	$(SDL_PATH)/src/scripts/planet/sim/PlanetShader.cpp \
	$(SDL_PATH)/src/scripts/RegulatorScript.cpp \
	$(SDL_PATH)/src/scripts/SpawnEntityFunctions.cpp \
	$(SDL_PATH)/src/scripts/TestScript.cpp \
	$(SDL_PATH)/src/scripts/vehicle/gfx/Puff.cpp \
	$(SDL_PATH)/src/scripts/vehicle/gfx/Thruster.cpp \
	$(SDL_PATH)/src/scripts/vehicle/sim/RoverThruster.cpp \
	$(SDL_PATH)/src/scripts/vehicle/sim/Thruster.cpp \
	$(SDL_PATH)/src/scripts/vehicle/sim/VehicleController.cpp \
	$(SDL_PATH)/src/sim/Controller.cpp \
	$(SDL_PATH)/src/sim/Engine.cpp \
	$(SDL_PATH)/src/sim/Entity.cpp \
	$(SDL_PATH)/src/sim/EntityFunctions.cpp \
	$(SDL_PATH)/src/sim/FrequencyEstimator.cpp \
	$(SDL_PATH)/src/sim/gravity.cpp \
	$(SDL_PATH)/src/sim/ObserverController.cpp \
	$(SDL_PATH)/src/sim/ObserverInput.cpp \
	$(SDL_PATH)/src/ipc/Fiber.cpp \
	$(SDL_PATH)/src/ipc/FiberPosix.cpp \
	$(SDL_PATH)/src/ipc/FiberWin.cpp \
	$(SDL_PATH)/src/smp/ReadersWriterMutex.cpp \
	$(SDL_PATH)/src/smp/scheduler.cpp \
	$(SDL_PATH)/src/smp/Semaphore.cpp \
	$(SDL_PATH)/src/smp/smp.cpp \
	$(SDL_PATH)/src/smp/Thread.cpp \
	$(SDL_PATH)/src/ipc/Uid.cpp

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
