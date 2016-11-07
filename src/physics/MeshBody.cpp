//
//  physics/MeshBody.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MeshBody.h"

#include "Engine.h"

#include "gfx/Debug.h"
#include "gfx/Mesh.h"
#include "gfx/PlainVertex.h"

#include "core/RosterObjectDefine.h"

#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// MeshBody

CRAG_ROSTER_OBJECT_DEFINE(
	MeshBody,
	1,
	Pool::CallBase<Body, & Body::PreTick>(Engine::GetPreTickRoster()),
	Pool::CallBase<Body, & Body::PostTick>(Engine::GetPostTickRoster()));

MeshBody::MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh, Scalar volume)
: Body(transformation, velocity, engine, engine.CreateMesh(nullptr))
, _mesh_data(dGeomTriMeshDataCreate())
, _bounding_radius(gfx::GetBoundingRadius(mesh))
{
	auto const & vertices = mesh.GetVertices();
	auto const & indices = mesh.GetIndices();
	
	dGeomTriMeshDataBuildSingle(_mesh_data,
		vertices.front().pos.GetAxes(), static_cast<int>(sizeof(Mesh::value_type)), static_cast<int>(vertices.size()),
		indices.data(), static_cast<int>(indices.size()), static_cast<int>(sizeof(Mesh::index_type)));

	dGeomTriMeshSetData(_collision_handle, _mesh_data);
	
#if defined(CRAG_DEBUG)
	_num_triangles = static_cast<int>(indices.size());
#endif

	// set mass
	if (_body_handle)
	{
		dMass m;

		// dMassSetTrimesh doesn't seem to like the ship mesh or I'd call it instead
		dMassSetSphere(& m, 1.f, Sphere3::Properties::RadiusFromVolume(volume));

		dBodySetMass (_body_handle, & m);
	}
}

MeshBody::MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh)
: MeshBody(transformation, velocity, engine, mesh, gfx::CalculateVolume(mesh))
{
}

MeshBody::~MeshBody()
{
	dGeomTriMeshDataDestroy(_mesh_data);
}

bool MeshBody::HandleCollision(Body & body, ContactFunction & contact_function)
{
	Sphere3 bounding_sphere(GetTranslation(), _bounding_radius);
	
	return body.HandleCollisionWithSolid(* this, bounding_sphere, contact_function);
}

void MeshBody::DebugDraw() const
{
#if defined(CRAG_DEBUG)
	for (auto triangle_index = 0; triangle_index != _num_triangles; ++ triangle_index)
	{
		std::array<dVector3, 3> triangle;
		dGeomTriMeshGetTriangle(
				GetCollisionHandle(),
				triangle_index,
				& triangle[0],
				& triangle[1],
				& triangle[2]);

#if defined(CRAG_COMPILER_MSVC) && defined(CRAG_DEBUG)
		// workaround for problem with VC2015
		// (see Convert in physics\defs.h)
		gfx::Debug::AddTriangle(Convert(triangle.data()));
#else
		gfx::Debug::AddTriangle(Convert(triangle));
#endif
	}
#endif
}
