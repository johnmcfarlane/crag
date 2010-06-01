/*
 *  Physics.cpp
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Physics.h"
#include "PhysicalBody.h"
#include "Entity.h"

#include "core/memory.h"

#include <algorithm>


//////////////////////////////////////////////////////////////////////
// dBodyID support functions

sim::Vector3 const & sim::GetPosition(dBodyID body)
{
	return * reinterpret_cast<sim::Vector3 const *>(dBodyGetPosition(body));
}

void sim::SetPosition(dBodyID body, sim::Vector3 const & pos)
{
	dBodySetPosition(body, pos.x, pos.y, pos.z);
}

void sim::GetRotation(dBodyID body, sim::Matrix4 & rot)
{
	dReal const * array = dBodyGetRotation(body);
	CopyArray(rot.GetArray(), array, 12);
	rot[3][0] = 0;
	rot[3][1] = 0;
	rot[3][2] = 0;
	rot[3][3] = 1;
}

void sim::SetRotation(dBodyID body, sim::Matrix4 const & matrix)
{
	dBodySetRotation(body, matrix.GetArray());
}

void sim::AddRelTorque(dBodyID body, sim::Vector3 const & torque)
{
	dBodyAddRelTorque(body, torque.x, torque.y, torque.z);
}

void sim::AddRelForce(dBodyID body, sim::Vector3 const & force)
{
	dBodyAddRelForce(body, force.x, force.y, force.z);
}

void sim::AddRelForceAtRelPos(dBodyID body, sim::Vector3 const & force, sim::Vector3 const & pos)
{
	dBodyAddForceAtRelPos(body, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

void sim::SetDamping(dBodyID body, float linear, float angular)
{
	dBodySetLinearDamping(body, linear);
	dBodySetAngularDamping(body, angular);
}


//////////////////////////////////////////////////////////////////////
// dGeomID support functions

sim::Vector3 const & sim::GetPosition(dGeomID geom)
{
	return * reinterpret_cast<sim::Vector3 const *>(dGeomGetPosition(geom));
}

void sim::SetPosition(dGeomID geom, sim::Vector3 const & pos)
{
	dGeomSetPosition(geom, pos.x, pos.y, pos.z);
}


//////////////////////////////////////////////////////////////////////
// Physics members

sim::Physics::Physics()
: world(dWorldCreate())
, space(dSimpleSpaceCreate(0))
, contact_joints(dJointGroupCreate(0))
//, formation_mesh(form::Manager::GetVertexBuffer())
, mesh_data(dGeomTriMeshDataCreate())
{
	dInitODE2(0);
	
	formation_geom = dCreateTriMesh (nullptr, mesh_data,
									 nullptr,	// dTriCallback *Callback,
									 nullptr,	// dTriArrayCallback * ArrayCallback,
									 nullptr);	// dTriRayCallback* RayCallback);
}


sim::Physics::~Physics()
{
	// TODO: Lots of tidy-up.
	sim::Physics::Destroy(formation_geom);
	dGeomTriMeshDataDestroy (mesh_data);	

	dSpaceDestroy(space);
	dWorldDestroy(world);
	dJointGroupDestroy(contact_joints);
}

form::Mesh & sim::Physics::GetFormationMesh()
{
	form::Mesh * broke = nullptr;
	return * broke;
}

dBodyID sim::Physics::CreateBody(UserDataType & data)
{
	dBodyID body = dBodyCreate(world);
	dBodySetData(body, & data);
	return body;
}

void sim::Physics::Destroy(dBodyID body)
{
	dBodyDestroy(body);
}

sim::UserDataType & sim::Physics::GetUserData(dBodyID body)
{
	return * static_cast<UserDataType *>(dBodyGetData(body));
}

dGeomID sim::Physics::CreateSphere(UserDataType & data, float /*density*/, float radius)
{
	dGeomID geom = dCreateSphere(space, radius);
	dGeomSetData(geom, & data);
	return geom;
}

void sim::Physics::Destroy(dGeomID geom)
{
	dGeomDestroy(geom);
}

sim::UserDataType & sim::Physics::GetUserData(dGeomID geom)
{
	return * static_cast<UserDataType *>(dGeomGetData(geom));
}

void sim::Physics::AddPhysicalBody(PhysicalBody & pb)
{
	Assert (! ContainsPhysicalBody(pb));
	physical_bodies.push_back(& pb);
	Assert (ContainsPhysicalBody(pb));
}

void sim::Physics::RemovePhysicalBody(PhysicalBody & pb)
{
	// Find the physical body in physical_bodies.
	PhysicalBodyContainer::iterator begin = physical_bodies.begin();
	PhysicalBodyContainer::iterator end = physical_bodies.end();
	PhysicalBodyContainer::iterator it = std::find(begin, end, & pb);
	Assert (it != end);

	// Remove it.
	physical_bodies.erase(it);
	
	// Make sure it wasn't double-entered.
	Assert (! ContainsPhysicalBody(pb));
}

bool sim::Physics::ContainsPhysicalBody(PhysicalBody const & pb) const
{ 
	PhysicalBodyContainer::const_iterator begin = physical_bodies.begin();
	PhysicalBodyContainer::const_iterator end = physical_bodies.end();
	return std::find(begin, end, & pb) != end;
}

void sim::Physics::Tick(double delta_time)
{
	// Detect / represent all collisions.
	CreateCollisions();
	
	// Tick physics (including acting upon collisions).
	dWorldQuickStep (world, delta_time);
	
	// Remove this tick's collision data.
	DestroyCollisions();
}

namespace sim {
static void OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2)
{
	Physics & physics = core::Singleton<Physics>::Get();
#if ! defined(NDEBUG)
	Physics & physicsAsData = * reinterpret_cast<Physics *>(data);
	Assert(& physics == & physicsAsData);
#endif
	
	Entity & entity1 = sim::Physics::GetUserData(geom1);
	Entity & entity2 = sim::Physics::GetUserData(geom2);
	
	PhysicalBody * body1 = entity1.GetPhysicalBody();
	PhysicalBody * body2 = entity2.GetPhysicalBody();
	
	if (body2 != nullptr && entity1.CustomCollision(* body2))
	{
		return;
	}
	
	if (body1 != nullptr && entity2.CustomCollision(* body1))
	{
		return;
	}
	
	physics.OnCollision(geom1, geom2);
}
}

void sim::Physics::CreateCollisions()
{
	// This basically calls a callback for all the geoms that are quite close.
	dSpaceCollide(space, reinterpret_cast<void *>(this), OnNearCollisionCallback);
}

void sim::Physics::DestroyCollisions()
{
	dJointGroupEmpty(contact_joints);
}

void sim::Physics::OnMeshCollision(dGeomID geom)
{
/*	form::VertexBuffer const & vertices = formation_mesh.GetVertices();
	const void * Vertices = & vertices[0].pos;
	int VertexStride = sizeof(vertices[0]);
	int VertexCount = vertices.GetMaxUsed(); 
	
	IndexBuffer const & indices = formation_mesh.GetIndices();
	const void * Indices = reinterpret_cast<void const *>(indices.GetArray());
	int IndexCount = indices.GetSize();
	int TriStride = sizeof(int) * 3;
	
	const void * Normals = & vertices[0].norm;
	
	dGeomTriMeshDataBuildSingle1(mesh_data,
								 Vertices, VertexStride, VertexCount, 
								 Indices, IndexCount, TriStride, 
								 Normals);
	
	sim::Physics::Get().OnCollision(geom, formation_geom);
	
	DrawMeshData(formation_geom, formation_mesh.GetNumPolys());*/
}

void sim::Physics::OnCollision(dGeomID geom1, dGeomID geom2)
{
	// No reason not to keep this nice and high; it's on the stack.
	// TODO: If cost of loop below gets too much, this array can be made a permanent member of this object and much of the init can be skipped.
	int const max_num_contacts = 128;
	dContact contacts [max_num_contacts];
	
	int num_contacts = dCollide (geom1, geom2, max_num_contacts, & contacts[0].geom, sizeof(* contacts));
	if (num_contacts == 0)
	{
		return;
	}
	
	// Time to increase max_num_contacts?
	Assert (num_contacts < max_num_contacts);
	
//	dBodyID body1 = dGeomGetBody(geom1);
//	dBodyID body2 = dGeomGetBody(geom2);
	
	dContact const * const end = contacts + num_contacts;
	for (dContact * it = contacts; it != end; ++ it)
	{
		// init the surface member of the contact
		dSurfaceParameters & surface = it->surface;
		surface.mode = dContactBounce | dContactSoftCFM;
		surface.mu = 1;				// used (by default)
		//surface.mu2 = 0;
  		surface.bounce = .5f;		// used
  		surface.bounce_vel = .1f;	// used
  		//surface.soft_erp = 0;
		//surface.soft_cfm = 0.001;
  		//surface.motion1 = 0;
		//surface.motion2 = 0;
		//surface.motionN = 0;
  		//surface.slip1 = 0;
		//surface.slip2 = 0;
		
		//it->fdir1[0] = 0;
		//it->fdir1[1] = 0;
		//it->fdir1[2] = 0;
		//it->fdir1[3] = 0;
		
		OnContact(it, geom1, geom2);
	}
}

void sim::Physics::OnContact(dContact * contact, dGeomID geom1, dGeomID geom2)
{
	dJointID c = dJointCreateContact (world, contact_joints, contact);
	dBodyID body1 = dGeomGetBody(geom1);
	dBodyID body2 = dGeomGetBody(geom2);
	dJointAttach (c, body1, body2);
}

void sim::Physics::DrawMeshData(dGeomID formation_geom, int num_triangles)
{
#if defined(DEBUG_GRAPHICS)
	for (int triangle = 0; triangle < num_triangles; ++ triangle)
	{
		dVector3 points[3];
		dGeomTriMeshGetTriangle (formation_geom, triangle, &points[0], &points[1], &points[2]);	
		Color4f col(1, 1, 1);
#if (0)
		sim::Vector3 s = sim::Vector3::Origin();
		for (int i = 0; i < 3; ++ i)
		{
			s.x += points[i][0];
			s.y += points[i][1];
			s.z += points[i][2];
		}
		s /= 3.f;
		DebugGraphics::AddPoint(s, col);
#else
		sim::Vector3 tri[3];
		for (int i = 0; i < 3; ++ i)
		{
			for (int axis = 0; axis < 3; ++ axis)
			{
				tri[i][axis] = points[i][axis];
			}
		}
		DebugGraphics::AddTriangle(tri[0], tri[1], tri[2], col);
#endif
	}
#endif
}

