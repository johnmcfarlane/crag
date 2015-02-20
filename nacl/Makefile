# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GNU Makefile based on shared rules provided by the Native Client SDK.
# See README.Makefiles for more details.

VALID_TOOLCHAINS := pnacl

NACL_DIR ?= $(abspath $(CURDIR))
NACL_SDK_ROOT ?= $(NACL_DIR)/nacl_sdk/$(SDK_NAME)
include $(NACL_SDK_ROOT)/tools/common.mk

TARGET = crag

# source directory
CRAG_SRC_DIR := $(NACL_DIR)/../src

# include flags
CRAG_INCLUDE_FLAGS := -I$(CRAG_SRC_DIR)
ODE_INCLUDE_FLAGS := -I$(NACL_DIR)/include
SDL_INCLUDE_FLAGS := -I$(NACL_DIR)/include/SDL2
NACL_INCLUDE_FLAGS := -I$(NACL_SDK_ROOT)/include
INCLUDE_FLAGS := $(ODE_INCLUDE_FLAGS) $(NACL_INCLUDE_FLAGS) $(SDL_INCLUDE_FLAGS) $(SDL_IMAGE_INCLUDE_FLAGS) $(CRAG_INCLUDE_FLAGS)

# link directories
LINK_DIR_FLAGS := -L$(NACL_DIR)/lib -L$(NACL_SDK_ROOT)/ports/lib/newlib_pnacl/$(CONFIG)

LIBS = ode SDL2_image png z SDL2 ppapi_simple SDL2main ppapi_simple nacl_io ppapi_gles2 ppapi_cpp ppapi pthread

FLAGS := $(INCLUDE_FLAGS)
CPP_WARNING_FLAGS := -Wall
CXXFLAGS := $(CXXFLAGS) $(FLAGS) -std=gnu++11 -pthread $(CPP_WARNING_FLAGS) -O2

SOURCES := \
	$(CRAG_SRC_DIR)/sim/EntityFunctions.cpp \
	$(CRAG_SRC_DIR)/sim/Entity.cpp \
	$(CRAG_SRC_DIR)/sim/Engine.cpp \
	$(CRAG_SRC_DIR)/sim/gravity.cpp \
	$(CRAG_SRC_DIR)/sim/Controller.cpp \
	$(CRAG_SRC_DIR)/entity/SpawnEntityFunctions.cpp \
	$(CRAG_SRC_DIR)/entity/sim/Contact.cpp \
	$(CRAG_SRC_DIR)/entity/sim/TouchObserverController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/Sensor.cpp \
	$(CRAG_SRC_DIR)/entity/sim/ObserverInput.cpp \
	$(CRAG_SRC_DIR)/entity/sim/RoverThruster.cpp \
	$(CRAG_SRC_DIR)/entity/sim/PlanetShader.cpp \
	$(CRAG_SRC_DIR)/entity/sim/MoonShader.cpp \
	$(CRAG_SRC_DIR)/entity/sim/UfoController2.cpp \
	$(CRAG_SRC_DIR)/entity/sim/AnimatController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/Thruster.cpp \
	$(CRAG_SRC_DIR)/entity/sim/VehicleController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/MouseObserverController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/AnimatThruster.cpp \
	$(CRAG_SRC_DIR)/entity/sim/VernierThruster.cpp \
	$(CRAG_SRC_DIR)/entity/sim/PlanetController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/UfoController1.cpp \
	$(CRAG_SRC_DIR)/entity/sim/CameraController.cpp \
	$(CRAG_SRC_DIR)/entity/sim/Genome.cpp \
	$(CRAG_SRC_DIR)/entity/sim/HoverThruster.cpp \
	$(CRAG_SRC_DIR)/entity/gfx/Puff.cpp \
	$(CRAG_SRC_DIR)/entity/gfx/Thruster.cpp \
	$(CRAG_SRC_DIR)/entity/gfx/Planet.cpp \
	$(CRAG_SRC_DIR)/entity/physics/PlanetBody.cpp \
	$(CRAG_SRC_DIR)/entity/SpawnPlayer.cpp \
	$(CRAG_SRC_DIR)/entity/SpawnSkybox.cpp \
	$(CRAG_SRC_DIR)/main.cpp \
	$(CRAG_SRC_DIR)/gfx/Frustum.cpp \
	$(CRAG_SRC_DIR)/gfx/glHelpers.cpp \
	$(CRAG_SRC_DIR)/gfx/Font.cpp \
	$(CRAG_SRC_DIR)/gfx/Quad.cpp \
	$(CRAG_SRC_DIR)/gfx/Image.cpp \
	$(CRAG_SRC_DIR)/gfx/RenderBuffer.cpp \
	$(CRAG_SRC_DIR)/gfx/Texture2d.cpp \
	$(CRAG_SRC_DIR)/gfx/PlainVertex.cpp \
	$(CRAG_SRC_DIR)/gfx/Engine.cpp \
	$(CRAG_SRC_DIR)/gfx/Uniform.cpp \
	$(CRAG_SRC_DIR)/gfx/LitVertex.cpp \
	$(CRAG_SRC_DIR)/gfx/Pov.cpp \
	$(CRAG_SRC_DIR)/gfx/Shader.cpp \
	$(CRAG_SRC_DIR)/gfx/Program.cpp \
	$(CRAG_SRC_DIR)/gfx/TextureCubeMap.cpp \
	$(CRAG_SRC_DIR)/gfx/RegisterResources.cpp \
	$(CRAG_SRC_DIR)/gfx/Debug.cpp \
	$(CRAG_SRC_DIR)/gfx/Scene.cpp \
	$(CRAG_SRC_DIR)/gfx/object/Ball.cpp \
	$(CRAG_SRC_DIR)/gfx/object/SearchLight.cpp \
	$(CRAG_SRC_DIR)/gfx/object/Skybox.cpp \
	$(CRAG_SRC_DIR)/gfx/object/Light.cpp \
	$(CRAG_SRC_DIR)/gfx/object/MeshObject.cpp \
	$(CRAG_SRC_DIR)/gfx/object/Surrounding.cpp \
	$(CRAG_SRC_DIR)/gfx/object/Object.cpp \
	$(CRAG_SRC_DIR)/form/CastRay.cpp \
	$(CRAG_SRC_DIR)/form/Mesh.cpp \
	$(CRAG_SRC_DIR)/form/Polyhedron.cpp \
	$(CRAG_SRC_DIR)/form/Formation.cpp \
	$(CRAG_SRC_DIR)/form/Node.cpp \
	$(CRAG_SRC_DIR)/form/PointBuffer.cpp \
	$(CRAG_SRC_DIR)/form/RayCastResult.cpp \
	$(CRAG_SRC_DIR)/form/Point.cpp \
	$(CRAG_SRC_DIR)/form/Engine.cpp \
	$(CRAG_SRC_DIR)/form/CalculateNodeScoreFunctor.cpp \
	$(CRAG_SRC_DIR)/form/QuaternaBuffer.cpp \
	$(CRAG_SRC_DIR)/form/Surrounding.cpp \
	$(CRAG_SRC_DIR)/form/Object.cpp \
	$(CRAG_SRC_DIR)/form/Scene.cpp \
	$(CRAG_SRC_DIR)/form/NodeBuffer.cpp \
	$(CRAG_SRC_DIR)/physics/SphereBody.cpp \
	$(CRAG_SRC_DIR)/physics/Location.cpp \
	$(CRAG_SRC_DIR)/physics/Body.cpp \
	$(CRAG_SRC_DIR)/physics/Engine.cpp \
	$(CRAG_SRC_DIR)/physics/CylinderBody.cpp \
	$(CRAG_SRC_DIR)/physics/GhostBody.cpp \
	$(CRAG_SRC_DIR)/physics/MeshBody.cpp \
	$(CRAG_SRC_DIR)/physics/PassiveLocation.cpp \
	$(CRAG_SRC_DIR)/physics/RayCast.cpp \
	$(CRAG_SRC_DIR)/physics/BoxBody.cpp \
	$(CRAG_SRC_DIR)/scripts/InitSpace.cpp \
	$(CRAG_SRC_DIR)/scripts/MainScript.cpp \
	$(CRAG_SRC_DIR)/scripts/MonitorOrigin.cpp \
	$(CRAG_SRC_DIR)/scripts/TestScript.cpp \
	$(CRAG_SRC_DIR)/scripts/GameScript.cpp \
	$(CRAG_SRC_DIR)/scripts/RegulatorScript.cpp \
	$(CRAG_SRC_DIR)/applet/Applet.cpp \
	$(CRAG_SRC_DIR)/applet/Engine.cpp \
	$(CRAG_SRC_DIR)/smp/smp.cpp \
	$(CRAG_SRC_DIR)/smp/Semaphore.cpp \
	$(CRAG_SRC_DIR)/smp/Thread.cpp \
	$(CRAG_SRC_DIR)/smp/ReadersWriterMutex.cpp \
	$(CRAG_SRC_DIR)/core/EventWatcher.cpp \
	$(CRAG_SRC_DIR)/core/debug.cpp \
	$(CRAG_SRC_DIR)/core/Roster.cpp \
	$(CRAG_SRC_DIR)/core/Resource.cpp \
	$(CRAG_SRC_DIR)/core/GlobalResourceManager.cpp \
	$(CRAG_SRC_DIR)/core/TypeId.cpp \
	$(CRAG_SRC_DIR)/core/ResourceManager.cpp \
	$(CRAG_SRC_DIR)/core/Statistics.cpp \
	$(CRAG_SRC_DIR)/core/app.cpp \
	$(CRAG_SRC_DIR)/core/memory.cpp \
	$(CRAG_SRC_DIR)/core/profile.cpp \
	$(CRAG_SRC_DIR)/core/Random.cpp \
	$(CRAG_SRC_DIR)/core/ConfigEntry.cpp \
	$(CRAG_SRC_DIR)/core/ConfigManager.cpp \
	$(CRAG_SRC_DIR)/pch.cpp \
	$(CRAG_SRC_DIR)/ipc/FiberPosix.cpp \
	$(CRAG_SRC_DIR)/ipc/Uid.cpp \
	$(CRAG_SRC_DIR)/ipc/FiberWin.cpp \
	$(CRAG_SRC_DIR)/ipc/Fiber.cpp \
	$(CRAG_SRC_DIR)/ipc/FiberAndroid.cpp \
	$(CRAG_SRC_DIR)/ipc/ListenerInterface.cpp

# Build rules generated by macros from common.mk:
# Overriden macro from NACL SDK to be able to customize the library search path -- Gabriel
# Specific Link Macro 
#
# $1 = Target Name
# $2 = List of inputs
# $3 = List of libs
# $4 = List of deps
# $5 = List of lib dirs
# $6 = Other Linker Args
#
# For debugging, we translate the pre-finalized .bc file.
#
define LINKER_RULE
all: $(1).pexe 
$(1)_x86_32.nexe : $(1).bc
	$(call LOG,TRANSLATE,$$@,$(PNACL_TRANSLATE) --allow-llvm-bitcode-input -arch x86-32 $$^ -o $$@)

$(1)_x86_64.nexe : $(1).bc
	$(call LOG,TRANSLATE,$$@,$(PNACL_TRANSLATE) --allow-llvm-bitcode-input -arch x86-64 $$^ -o $$@)

$(1)_arm.nexe : $(1).bc
	$(call LOG,TRANSLATE,$$@,$(PNACL_TRANSLATE) --allow-llvm-bitcode-input -arch arm $$^ -o $$@)

$(1).pexe: $(1).bc
	$(call LOG,FINALIZE,$$@,$(PNACL_FINALIZE) -o $$@ $$^)

$(1).bc: $(2) $(foreach dep,$(4),$(STAMPDIR)/$(dep).stamp)
	$(call LOG,LINK,$$@,$(PNACL_LINK) -o $$@ $(2) $(PNACL_LDFLAGS) --pnacl-driver-verbose $(foreach path,$(5),-L$(path)/pnacl/$(CONFIG)) $(LINK_DIR_FLAGS) $(foreach lib,$(3),-l$(lib)) $(6))
endef

$(foreach src,$(SOURCES),$(eval $(call COMPILE_RULE,$(src),$(CXXFLAGS))))

ifeq ($(CONFIG),Release)
$(eval $(call LINK_RULE,$(TARGET)_unstripped,$(SOURCES),$(LIBS)))
$(eval $(call STRIP_RULE,$(TARGET),$(TARGET)_unstripped))
else
$(eval $(call LINK_RULE,$(TARGET),$(SOURCES),$(LIBS)))
endif

$(eval $(call NMF_RULE,$(TARGET),))
