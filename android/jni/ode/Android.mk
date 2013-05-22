LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ode

ODE_PATH := ../../../dependencies/ode
ODE_SOURCE := $(ODE_PATH)/ode/src
ODE_INCLUDE := $(ODE_PATH)/include

OU_PATH := $(ODE_PATH)/ou
OU_SRC_PATH := $(OU_PATH)/src/ou
OU_INCLUDE := $(OU_PATH)/include

CCD_PATH := $(ODE_PATH)/libccd
CCD_SOURCE := $(CCD_PATH)/src
CCD_INCLUDE := $(CCD_PATH)/src

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(ODE_SOURCE) \
	$(LOCAL_PATH)/$(ODE_INCLUDE) \
	$(LOCAL_PATH)/$(OU_INCLUDE) \
	$(LOCAL_PATH)/$(CCD_INCLUDE)

# Add your application source files here...
LOCAL_SRC_FILES := $(ODE_SOURCE)/array.cpp \
	$(ODE_SOURCE)/box.cpp \
	$(ODE_SOURCE)/capsule.cpp \
	$(ODE_SOURCE)/collision_cylinder_box.cpp \
	$(ODE_SOURCE)/collision_cylinder_plane.cpp \
	$(ODE_SOURCE)/collision_cylinder_sphere.cpp \
	$(ODE_SOURCE)/collision_cylinder_trimesh.cpp \
	$(ODE_SOURCE)/collision_kernel.cpp \
	$(ODE_SOURCE)/collision_libccd.cpp \
	$(ODE_SOURCE)/collision_quadtreespace.cpp \
	$(ODE_SOURCE)/collision_sapspace.cpp \
	$(ODE_SOURCE)/collision_space.cpp \
	$(ODE_SOURCE)/collision_transform.cpp \
	$(ODE_SOURCE)/collision_trimesh_box.cpp \
	$(ODE_SOURCE)/collision_trimesh_ccylinder.cpp \
	$(ODE_SOURCE)/collision_trimesh_disabled.cpp \
	$(ODE_SOURCE)/collision_trimesh_distance.cpp \
	$(ODE_SOURCE)/collision_trimesh_gimpact.cpp \
	$(ODE_SOURCE)/collision_trimesh_opcode.cpp \
	$(ODE_SOURCE)/collision_trimesh_plane.cpp \
	$(ODE_SOURCE)/collision_trimesh_ray.cpp \
	$(ODE_SOURCE)/collision_trimesh_sphere.cpp \
	$(ODE_SOURCE)/collision_trimesh_trimesh.cpp \
	$(ODE_SOURCE)/collision_trimesh_trimesh_new.cpp \
	$(ODE_SOURCE)/collision_util.cpp \
	$(ODE_SOURCE)/convex.cpp \
	$(ODE_SOURCE)/cylinder.cpp \
	$(ODE_SOURCE)/error.cpp \
	$(ODE_SOURCE)/export-dif.cpp \
	$(ODE_SOURCE)/heightfield.cpp \
	$(ODE_SOURCE)/lcp.cpp \
	$(ODE_SOURCE)/mass.cpp \
	$(ODE_SOURCE)/mat.cpp \
	$(ODE_SOURCE)/matrix.cpp \
	$(ODE_SOURCE)/memory.cpp \
	$(ODE_SOURCE)/misc.cpp \
	$(ODE_SOURCE)/obstack.cpp \
	$(ODE_SOURCE)/ode.cpp \
	$(ODE_SOURCE)/odeinit.cpp \
	$(ODE_SOURCE)/odemath.cpp \
	$(ODE_SOURCE)/odeou.cpp \
	$(ODE_SOURCE)/odetls.cpp \
	$(ODE_SOURCE)/plane.cpp \
	$(ODE_SOURCE)/quickstep.cpp \
	$(ODE_SOURCE)/ray.cpp \
	$(ODE_SOURCE)/rotation.cpp \
	$(ODE_SOURCE)/sphere.cpp \
	$(ODE_SOURCE)/step.cpp \
	$(ODE_SOURCE)/timer.cpp \
	$(ODE_SOURCE)/util.cpp \
	$(ODE_SOURCE)/joints/amotor.cpp \
	$(ODE_SOURCE)/joints/ball.cpp \
	$(ODE_SOURCE)/joints/contact.cpp \
	$(ODE_SOURCE)/joints/fixed.cpp \
	$(ODE_SOURCE)/joints/hinge2.cpp \
	$(ODE_SOURCE)/joints/hinge.cpp \
	$(ODE_SOURCE)/joints/joint.cpp \
	$(ODE_SOURCE)/joints/lmotor.cpp \
	$(ODE_SOURCE)/joints/null.cpp \
	$(ODE_SOURCE)/joints/piston.cpp \
	$(ODE_SOURCE)/joints/plane2d.cpp \
	$(ODE_SOURCE)/joints/pr.cpp \
	$(ODE_SOURCE)/joints/pu.cpp \
	$(ODE_SOURCE)/joints/slider.cpp \
	$(ODE_SOURCE)/joints/universal.cpp \
	$(ODE_SOURCE)/fastdot.c \
	$(ODE_SOURCE)/fastldlt.c \
	$(ODE_SOURCE)/fastlsolve.c \
	$(ODE_SOURCE)/fastltsolve.c \
	$(ODE_SOURCE)/nextafterf.c \
	$(OU_SRC_PATH)/atomic.cpp \
	$(OU_SRC_PATH)/customization.cpp \
	$(OU_SRC_PATH)/malloc.cpp \
	$(OU_SRC_PATH)/threadlocalstorage.cpp \
	$(CCD_SOURCE)/alloc.c \
	$(CCD_SOURCE)/ccd.c \
	$(CCD_SOURCE)/mpr.c \
	$(CCD_SOURCE)/polytope.c \
	$(CCD_SOURCE)/support.c \
	$(CCD_SOURCE)/vec3.c

LOCAL_CPPFLAGS := -std=c++11 -O2 -pthread -Wall -Wfatal-errors -fno-rtti -fno-exceptions -D__ODE__ -D_OU_NAMESPACE=odeou -D__STRICT_ANSI__ -DNDEBUG -DdSINGLE -DCCD_SINGLE
LOCAL_CFLAGS := -O2 -pthread -Wall -Wfatal-errors -D__STRICT_ANSI__ -DNDEBUG -DdSINGLE -DCCD_SINGLE

include $(BUILD_SHARED_LIBRARY)
