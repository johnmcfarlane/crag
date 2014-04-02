LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
ODE_PATH := 
CRAG_PATH := ../../../src

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
	$(LOCAL_PATH)/../ode/include \
	$(LOCAL_PATH)/$(CRAG_PATH)

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
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
	$(CRAG_PATH)/core/Resource.cpp \
	$(CRAG_PATH)/core/ResourceManager.cpp \
	$(CRAG_PATH)/core/Roster.cpp \
	$(CRAG_PATH)/core/Statistics.cpp \
	$(CRAG_PATH)/core/TypeId.cpp \
	$(CRAG_PATH)/entity/gfx/Planet.cpp \
	$(CRAG_PATH)/entity/gfx/Puff.cpp \
	$(CRAG_PATH)/entity/gfx/Thruster.cpp \
	$(CRAG_PATH)/entity/physics/PlanetBody.cpp \
	$(CRAG_PATH)/entity/sim/AnimatController.cpp \
	$(CRAG_PATH)/entity/sim/AnimatThruster.cpp \
	$(CRAG_PATH)/entity/sim/CameraController.cpp \
	$(CRAG_PATH)/entity/sim/Contact.cpp \
	$(CRAG_PATH)/entity/sim/HoverThruster.cpp \
	$(CRAG_PATH)/entity/sim/Genome.cpp \
	$(CRAG_PATH)/entity/sim/MoonShader.cpp \
	$(CRAG_PATH)/entity/sim/MouseObserverController.cpp \
	$(CRAG_PATH)/entity/sim/ObserverInput.cpp \
	$(CRAG_PATH)/entity/sim/PlanetController.cpp \
	$(CRAG_PATH)/entity/sim/PlanetShader.cpp \
	$(CRAG_PATH)/entity/sim/RoverThruster.cpp \
	$(CRAG_PATH)/entity/sim/Sensor.cpp \
	$(CRAG_PATH)/entity/sim/Thruster.cpp \
	$(CRAG_PATH)/entity/sim/TouchObserverController.cpp \
	$(CRAG_PATH)/entity/sim/UfoController.cpp \
	$(CRAG_PATH)/entity/sim/VehicleController.cpp \
	$(CRAG_PATH)/entity/sim/VernierThruster.cpp \
	$(CRAG_PATH)/entity/SpawnEntityFunctions.cpp \
	$(CRAG_PATH)/entity/SpawnSkybox.cpp \
	$(CRAG_PATH)/form/CalculateNodeScoreFunctor.cpp \
	$(CRAG_PATH)/form/CastRay.cpp \
	$(CRAG_PATH)/form/Engine.cpp \
	$(CRAG_PATH)/form/Formation.cpp \
	$(CRAG_PATH)/form/Mesh.cpp \
	$(CRAG_PATH)/form/NodeBuffer.cpp \
	$(CRAG_PATH)/form/Node.cpp \
	$(CRAG_PATH)/form/PointBuffer.cpp \
	$(CRAG_PATH)/form/Point.cpp \
	$(CRAG_PATH)/form/QuaternaBuffer.cpp \
	$(CRAG_PATH)/form/RayCastResult.cpp \
	$(CRAG_PATH)/form/Polyhedron.cpp \
	$(CRAG_PATH)/form/Scene.cpp \
	$(CRAG_PATH)/form/Surrounding.cpp \
	$(CRAG_PATH)/gfx/Debug.cpp \
	$(CRAG_PATH)/gfx/Engine.cpp \
	$(CRAG_PATH)/gfx/Font.cpp \
	$(CRAG_PATH)/gfx/FrameBuffer.cpp \
	$(CRAG_PATH)/gfx/glHelpers.cpp \
	$(CRAG_PATH)/gfx/Image.cpp \
	$(CRAG_PATH)/gfx/object/Ball.cpp \
	$(CRAG_PATH)/gfx/object/Surrounding.cpp \
	$(CRAG_PATH)/gfx/object/LeafNode.cpp \
	$(CRAG_PATH)/gfx/object/Light.cpp \
	$(CRAG_PATH)/gfx/object/Object.cpp \
	$(CRAG_PATH)/gfx/object/RootNode.cpp \
	$(CRAG_PATH)/gfx/object/Skybox.cpp \
	$(CRAG_PATH)/gfx/object/MeshObject.cpp \
	$(CRAG_PATH)/gfx/PlainVertex.cpp \
	$(CRAG_PATH)/gfx/Pov.cpp \
	$(CRAG_PATH)/gfx/Program.cpp \
	$(CRAG_PATH)/gfx/Quad.cpp \
	$(CRAG_PATH)/gfx/RegisterResources.cpp \
	$(CRAG_PATH)/gfx/RenderBuffer.cpp \
	$(CRAG_PATH)/gfx/Scene.cpp \
	$(CRAG_PATH)/gfx/Shader.cpp \
	$(CRAG_PATH)/gfx/Texture.cpp \
	$(CRAG_PATH)/gfx/LitVertex.cpp \
	$(CRAG_PATH)/gfx/Uniform.cpp \
	$(CRAG_PATH)/main.cpp \
	$(CRAG_PATH)/pch.cpp \
	$(CRAG_PATH)/physics/Body.cpp \
	$(CRAG_PATH)/physics/BoxBody.cpp \
	$(CRAG_PATH)/physics/CylinderBody.cpp \
	$(CRAG_PATH)/physics/Engine.cpp \
	$(CRAG_PATH)/physics/GhostBody.cpp \
	$(CRAG_PATH)/physics/Location.cpp \
	$(CRAG_PATH)/physics/MeshBody.cpp \
	$(CRAG_PATH)/physics/PassiveLocation.cpp \
	$(CRAG_PATH)/physics/RayCast.cpp \
	$(CRAG_PATH)/physics/SphereBody.cpp \
	$(CRAG_PATH)/scripts/GameScript.cpp \
	$(CRAG_PATH)/scripts/MainScript.cpp \
	$(CRAG_PATH)/scripts/MonitorOrigin.cpp \
	$(CRAG_PATH)/scripts/RegulatorScript.cpp \
	$(CRAG_PATH)/scripts/TestScript.cpp \
	$(CRAG_PATH)/sim/Controller.cpp \
	$(CRAG_PATH)/sim/Engine.cpp \
	$(CRAG_PATH)/sim/Entity.cpp \
	$(CRAG_PATH)/sim/EntityFunctions.cpp \
	$(CRAG_PATH)/sim/gravity.cpp \
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
	LOCAL_CPPFLAGS := -std=c++11 -g -pthread -DPROFILE -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -Wno-unused-function -Wno-attributes -fno-rtti -fno-exceptions
else
	NDK_DEBUG=0
	LOCAL_CPPFLAGS := -std=c++11 -O2 -pthread -DNDEBUG -D__STRICT_ANSI__ -Wall -Wextra -Wfatal-errors -Wno-unused-function -Wno-attributes -fno-rtti -fno-exceptions
endif

LOCAL_SHARED_LIBRARIES := SDL2 ode

LOCAL_LDLIBS := -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
