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

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// MeshBody

MeshBody::MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh)
: Body(transformation, velocity, engine, engine.CreateMesh(nullptr))
, _mesh_data(dGeomTriMeshDataCreate())
, _bounding_radius(gfx::GetBoundingRadius(mesh))
{
	auto const & vertices = mesh.GetVertices();
	auto const & indices = mesh.GetIndices();
	
	dGeomTriMeshDataBuildSingle(_mesh_data,
		vertices.front().pos.GetAxes(), sizeof(Mesh::VertexType), vertices.size(),
		indices.data(), indices.size(), sizeof(Mesh::IndexType));

	dGeomTriMeshSetData(_collision_handle, _mesh_data);
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
