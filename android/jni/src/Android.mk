LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
ODE_PATH := ../../../dependencies/ode
CRAG_PATH := ../../../src

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(ODE_PATH)/include \
	$(LOCAL_PATH)/$(CRAG_PATH)

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	$(CRAG_PATH)/applet/Applet.cpp \
	$(CRAG_PATH)/applet/Engine.cpp \
	$(CRAG_PATH)/core/app.cpp \
	$(CRAG_PATH)/core/ConfigEntry.cpp \
	$(CRAG_PATH)/core/ConfigManager.cpp \
	$(CRAG_PATH)/core/debug.cpp \
	$(CRAG_PATH)/core/EventWatcher.cpp \
	$(CRAG_PATH)/core/memory.cpp \
	$(CRAG_PATH)/core/profile.cpp \
	$(CRAG_PATH)/core/Random.cpp \
	$(CRAG_PATH)/core/Roster.cpp \
	$(CRAG_PATH)/core/Statistics.cpp \
	$(CRAG_PATH)/form/Engine.cpp \
	$(CRAG_PATH)/form/Formation.cpp \
	$(CRAG_PATH)/form/node/CalculateNodeScoreFunctor.cpp \
	$(CRAG_PATH)/form/node/NodeBuffer.cpp \
	$(CRAG_PATH)/form/node/Node.cpp \
	$(CRAG_PATH)/form/node/PointBuffer.cpp \
	$(CRAG_PATH)/form/node/Point.cpp \
	$(CRAG_PATH)/form/node/RootNode.cpp \
	$(CRAG_PATH)/form/scene/MeshBufferObject.cpp \
	$(CRAG_PATH)/form/scene/Mesh.cpp \
	$(CRAG_PATH)/form/scene/Polyhedron.cpp \
	$(CRAG_PATH)/form/scene/Scene.cpp \
	$(CRAG_PATH)/form/scene/VertexBuffer.cpp \
	$(CRAG_PATH)/form/scene/Vertex.cpp \
	$(CRAG_PATH)/gfx/Cuboid.cpp \
	$(CRAG_PATH)/gfx/Debug.cpp \
	$(CRAG_PATH)/gfx/Engine.cpp \
	$(CRAG_PATH)/gfx/Fence.cpp \
	$(CRAG_PATH)/gfx/Font.cpp \
	$(CRAG_PATH)/gfx/FrameBuffer.cpp \
	$(CRAG_PATH)/gfx/glHelpers.cpp \
	$(CRAG_PATH)/gfx/Image.cpp \
	$(CRAG_PATH)/gfx/IndexBuffer.cpp \
	$(CRAG_PATH)/gfx/Layer.cpp \
	$(CRAG_PATH)/gfx/object/Ball.cpp \
	$(CRAG_PATH)/gfx/object/Box.cpp \
	$(CRAG_PATH)/gfx/object/FormationMesh.cpp \
	$(CRAG_PATH)/gfx/object/LeafNode.cpp \
	$(CRAG_PATH)/gfx/object/Light.cpp \
	$(CRAG_PATH)/gfx/object/Object.cpp \
	$(CRAG_PATH)/gfx/object/RootNode.cpp \
	$(CRAG_PATH)/gfx/object/Skybox.cpp \
	$(CRAG_PATH)/gfx/Pov.cpp \
	$(CRAG_PATH)/gfx/Program.cpp \
	$(CRAG_PATH)/gfx/Quad.cpp \
	$(CRAG_PATH)/gfx/RenderBuffer.cpp \
	$(CRAG_PATH)/gfx/ResourceManager.cpp \
	$(CRAG_PATH)/gfx/Scene.cpp \
	$(CRAG_PATH)/gfx/Shader.cpp \
	$(CRAG_PATH)/gfx/Texture.cpp \
	$(CRAG_PATH)/main.cpp \
	$(CRAG_PATH)/pch.cpp \
	$(CRAG_PATH)/physics/Body.cpp \
	$(CRAG_PATH)/physics/BoxBody.cpp \
	$(CRAG_PATH)/physics/Engine.cpp \
	$(CRAG_PATH)/physics/FixedLocation.cpp \
	$(CRAG_PATH)/physics/IntersectionFunctor.cpp \
	$(CRAG_PATH)/physics/Location.cpp \
	$(CRAG_PATH)/physics/SphericalBody.cpp \
	$(CRAG_PATH)/scripts/MainScript.cpp \
	$(CRAG_PATH)/scripts/MonitorOrigin.cpp \
	$(CRAG_PATH)/scripts/planet/gfx/Planet.cpp \
	$(CRAG_PATH)/scripts/planet/physics/PlanetBody.cpp \
	$(CRAG_PATH)/scripts/planet/sim/MoonShader.cpp \
	$(CRAG_PATH)/scripts/planet/sim/PlanetController.cpp \
	$(CRAG_PATH)/scripts/planet/sim/PlanetShader.cpp \
	$(CRAG_PATH)/scripts/RegulatorScript.cpp \
	$(CRAG_PATH)/scripts/SpawnEntityFunctions.cpp \
	$(CRAG_PATH)/scripts/TestScript.cpp \
	$(CRAG_PATH)/scripts/vehicle/gfx/Puff.cpp \
	$(CRAG_PATH)/scripts/vehicle/gfx/Thruster.cpp \
	$(CRAG_PATH)/scripts/vehicle/sim/RoverThruster.cpp \
	$(CRAG_PATH)/scripts/vehicle/sim/Thruster.cpp \
	$(CRAG_PATH)/scripts/vehicle/sim/VehicleController.cpp \
	$(CRAG_PATH)/sim/Controller.cpp \
	$(CRAG_PATH)/sim/Engine.cpp \
	$(CRAG_PATH)/sim/Entity.cpp \
	$(CRAG_PATH)/sim/EntityFunctions.cpp \
	$(CRAG_PATH)/sim/FrequencyEstimator.cpp \
	$(CRAG_PATH)/sim/gravity.cpp \
	$(CRAG_PATH)/sim/MouseObserverController.cpp \
	$(CRAG_PATH)/sim/ObserverInput.cpp \
	$(CRAG_PATH)/sim/TouchObserverController.cpp \
	$(CRAG_PATH)/ipc/Fiber.cpp \
	$(CRAG_PATH)/ipc/FiberAndroid.cpp \
	$(CRAG_PATH)/ipc/FiberPosix.cpp \
	$(CRAG_PATH)/ipc/FiberWin.cpp \
	$(CRAG_PATH)/ipc/ListenerInterface.cpp \
	$(CRAG_PATH)/smp/ReadersWriterMutex.cpp \
	$(CRAG_PATH)/smp/scheduler.cpp \
	$(CRAG_PATH)/smp/Semaphore.cpp \
	$(CRAG_PATH)/smp/smp.cpp \
	$(CRAG_PATH)/smp/Thread.cpp \
	$(CRAG_PATH)/ipc/Uid.cpp \

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

ifeq ($(APP_OPTIM),debug)
	NDK_DEBUG=1
	LOCAL_CPPFLAGS := -std=c++11 -g -pthread -DPROFILE -D__STRICT_ANSI__ -DdSINGLE -Wall -Wextra -Wfatal-errors -fno-rtti -fno-exceptions
else
	NDK_DEBUG=0
	LOCAL_CPPFLAGS := -std=c++11 -O2 -pthread -DNDEBUG -D__STRICT_ANSI__ -DdSINGLE -Wall -Wextra -Wfatal-errors -fno-rtti -fno-exceptions
endif

LOCAL_SHARED_LIBRARIES := SDL2 ode

LOCAL_LDLIBS := -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
