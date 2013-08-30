LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ode

ODE_PATH := ../../../dependencies/ode

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(ODE_PATH)/ode/src \
	$(LOCAL_PATH)/$(ODE_PATH)/include \
	$(LOCAL_PATH)/$(ODE_PATH)/ou/include \
	$(LOCAL_PATH)/$(ODE_PATH)/libccd/src \
	$(LOCAL_PATH)/$(ODE_PATH)/OPCODE

# Add your application source files here...
LOCAL_SRC_FILES := $(ODE_PATH)/ode/src/array.cpp \
	$(ODE_PATH)/ode/src/box.cpp \
	$(ODE_PATH)/ode/src/capsule.cpp \
	$(ODE_PATH)/ode/src/collision_cylinder_box.cpp \
	$(ODE_PATH)/ode/src/collision_cylinder_plane.cpp \
	$(ODE_PATH)/ode/src/collision_cylinder_sphere.cpp \
	$(ODE_PATH)/ode/src/collision_cylinder_trimesh.cpp \
	$(ODE_PATH)/ode/src/collision_kernel.cpp \
	$(ODE_PATH)/ode/src/collision_libccd.cpp \
	$(ODE_PATH)/ode/src/collision_quadtreespace.cpp \
	$(ODE_PATH)/ode/src/collision_sapspace.cpp \
	$(ODE_PATH)/ode/src/collision_space.cpp \
	$(ODE_PATH)/ode/src/collision_transform.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_box.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_ccylinder.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_disabled.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_distance.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_gimpact.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_opcode.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_plane.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_ray.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_sphere.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_trimesh.cpp \
	$(ODE_PATH)/ode/src/collision_trimesh_trimesh_new.cpp \
	$(ODE_PATH)/ode/src/collision_util.cpp \
	$(ODE_PATH)/ode/src/convex.cpp \
	$(ODE_PATH)/ode/src/cylinder.cpp \
	$(ODE_PATH)/ode/src/error.cpp \
	$(ODE_PATH)/ode/src/export-dif.cpp \
	$(ODE_PATH)/ode/src/heightfield.cpp \
	$(ODE_PATH)/ode/src/lcp.cpp \
	$(ODE_PATH)/ode/src/mass.cpp \
	$(ODE_PATH)/ode/src/mat.cpp \
	$(ODE_PATH)/ode/src/matrix.cpp \
	$(ODE_PATH)/ode/src/memory.cpp \
	$(ODE_PATH)/ode/src/misc.cpp \
	$(ODE_PATH)/ode/src/obstack.cpp \
	$(ODE_PATH)/ode/src/ode.cpp \
	$(ODE_PATH)/ode/src/odeinit.cpp \
	$(ODE_PATH)/ode/src/odemath.cpp \
	$(ODE_PATH)/ode/src/odeou.cpp \
	$(ODE_PATH)/ode/src/odetls.cpp \
	$(ODE_PATH)/ode/src/plane.cpp \
	$(ODE_PATH)/ode/src/quickstep.cpp \
	$(ODE_PATH)/ode/src/ray.cpp \
	$(ODE_PATH)/ode/src/rotation.cpp \
	$(ODE_PATH)/ode/src/sphere.cpp \
	$(ODE_PATH)/ode/src/step.cpp \
	$(ODE_PATH)/ode/src/timer.cpp \
	$(ODE_PATH)/ode/src/util.cpp \
	$(ODE_PATH)/ode/src/joints/amotor.cpp \
	$(ODE_PATH)/ode/src/joints/ball.cpp \
	$(ODE_PATH)/ode/src/joints/contact.cpp \
	$(ODE_PATH)/ode/src/joints/fixed.cpp \
	$(ODE_PATH)/ode/src/joints/hinge2.cpp \
	$(ODE_PATH)/ode/src/joints/hinge.cpp \
	$(ODE_PATH)/ode/src/joints/joint.cpp \
	$(ODE_PATH)/ode/src/joints/lmotor.cpp \
	$(ODE_PATH)/ode/src/joints/null.cpp \
	$(ODE_PATH)/ode/src/joints/piston.cpp \
	$(ODE_PATH)/ode/src/joints/plane2d.cpp \
	$(ODE_PATH)/ode/src/joints/pr.cpp \
	$(ODE_PATH)/ode/src/joints/pu.cpp \
	$(ODE_PATH)/ode/src/joints/slider.cpp \
	$(ODE_PATH)/ode/src/joints/universal.cpp \
	$(ODE_PATH)/ode/src/fastdot.c \
	$(ODE_PATH)/ode/src/fastldlt.c \
	$(ODE_PATH)/ode/src/fastlsolve.c \
	$(ODE_PATH)/ode/src/fastltsolve.c \
	$(ODE_PATH)/ode/src/nextafterf.c \
	$(ODE_PATH)/ou/src/ou/atomic.cpp \
	$(ODE_PATH)/ou/src/ou/customization.cpp \
	$(ODE_PATH)/ou/src/ou/malloc.cpp \
	$(ODE_PATH)/ou/src/ou/threadlocalstorage.cpp \
	$(ODE_PATH)/libccd/src/alloc.c \
	$(ODE_PATH)/libccd/src/ccd.c \
	$(ODE_PATH)/libccd/src/mpr.c \
	$(ODE_PATH)/libccd/src/polytope.c \
	$(ODE_PATH)/libccd/src/support.c \
	$(ODE_PATH)/libccd/src/vec3.c \
	$(ODE_PATH)/OPCODE/OPC_AABBCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_AABBTree.cpp \
	$(ODE_PATH)/OPCODE/OPC_BaseModel.cpp \
	$(ODE_PATH)/OPCODE/OPC_Collider.cpp \
	$(ODE_PATH)/OPCODE/OPC_Common.cpp \
	$(ODE_PATH)/OPCODE/OPC_HybridModel.cpp \
	$(ODE_PATH)/OPCODE/OPC_LSSCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_MeshInterface.cpp \
	$(ODE_PATH)/OPCODE/OPC_Model.cpp \
	$(ODE_PATH)/OPCODE/OPC_OBBCollider.cpp \
	$(ODE_PATH)/OPCODE/Opcode.cpp \
	$(ODE_PATH)/OPCODE/OPC_OptimizedTree.cpp \
	$(ODE_PATH)/OPCODE/OPC_Picking.cpp \
	$(ODE_PATH)/OPCODE/OPC_PlanesCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_RayCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_SphereCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_TreeBuilders.cpp \
	$(ODE_PATH)/OPCODE/OPC_TreeCollider.cpp \
	$(ODE_PATH)/OPCODE/OPC_VolumeCollider.cpp \
	$(ODE_PATH)/OPCODE/StdAfx.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceAABB.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceContainer.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceHPoint.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceIndexedTriangle.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceMatrix3x3.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceMatrix4x4.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceOBB.cpp \
	$(ODE_PATH)/OPCODE/Ice/IcePlane.cpp \
	$(ODE_PATH)/OPCODE/Ice/IcePoint.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceRandom.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceRay.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceRevisitedRadix.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceSegment.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceTriangle.cpp \
	$(ODE_PATH)/OPCODE/Ice/IceUtils.cpp

LOCAL_CPPFLAGS := -std=c++11 -O2 -pthread -Wall -Wfatal-errors -fno-rtti -fno-exceptions -D__ODE__ -D_OU_NAMESPACE=odeou -D__STRICT_ANSI__ -DNDEBUG -DdSINGLE -DCCD_SINGLE -DdTRIMESH_ENABLED -DdTRIMESH_OPCODE
LOCAL_CFLAGS := -O2 -pthread -Wall -Wfatal-errors -D__STRICT_ANSI__ -DNDEBUG -DdSINGLE -DCCD_SINGLE -DdTRIMESH_ENABLED -DdTRIMESH_OPCODE

include $(BUILD_SHARED_LIBRARY)
