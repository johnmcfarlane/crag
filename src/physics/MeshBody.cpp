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

#include "gfx/Mesh.h"
#include "gfx/PlainVertex.h"

#include <ode/objects.h>
#include <gfx/Debug.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// MeshBody

MeshBody::MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh, Scalar volume)
: Body(transformation, velocity, engine, engine.CreateMesh(nullptr))
, _mesh_data(dGeomTriMeshDataCreate())
, _bounding_radius(gfx::GetBoundingRadius(mesh))
{
	auto const & vertices = mesh.GetVertices();
	auto const & indices = mesh.GetIndices();
	
	dGeomTriMeshDataBuildSingle(_mesh_data,
		vertices.front().pos.GetAxes(), sizeof(Mesh::value_type), vertices.size(),
		indices.data(), indices.size(), sizeof(Mesh::index_type));

	dGeomTriMeshSetData(_collision_handle, _mesh_data);
	
#if defined(CRAG_DEBUG)
	_num_triangles = indices.size();
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

bool MeshBody::OnCollision(Body & body, ContactInterface & contact_interface)
{
	Sphere3 bounding_sphere(GetTranslation(), _bounding_radius);
	
	return body.OnCollisionWithSolid(* this, bounding_sphere, contact_interface);
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

		gfx::Debug::AddTriangle(Convert(triangle));
	}
#endif
}
