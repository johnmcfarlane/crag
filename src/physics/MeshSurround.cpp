//
//  MeshSurround.cpp
//  crag
//
//  Created by John on 2013-08-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MeshSurround.h"

#if ! defined(NDEBUG)
//#define DEBUG_TRIANGLES
#endif

#if defined(DEBUG_TRIANGLES)
#include "gfx/Debug.h"
#include "core/Random.h"
#endif

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// MeshSurround member definition

MeshSurround::MeshSurround()
: _mesh_data(dGeomTriMeshDataCreate())
, _collision_handle(dCreateTriMesh(nullptr, _mesh_data, nullptr, nullptr, nullptr))
{
}

MeshSurround::MeshSurround(MeshSurround && rhs)
: _mesh_data(rhs._mesh_data)
, _collision_handle(rhs._collision_handle)
, _vertices(std::move(rhs._vertices))
, _triangle_indices(std::move(rhs._triangle_indices))
, _normals(std::move(rhs._normals))
{
	rhs._mesh_data = 0;
	rhs._collision_handle = 0;
}

MeshSurround::~MeshSurround()
{
	ASSERT((! _collision_handle) == (! _mesh_data));
	if (_collision_handle)
	{
		ASSERT(dGeomTriMeshGetData(_collision_handle) == _mesh_data);
		dGeomDestroy(_collision_handle);
		dGeomTriMeshDataDestroy(_mesh_data);
	}
}

CollisionHandle MeshSurround::GetCollisionHandle() const
{
	return _collision_handle;
}

void MeshSurround::Enable()
{
	dGeomEnable(_collision_handle);
}

void MeshSurround::Disable()
{
	dGeomDisable(_collision_handle);
}

void MeshSurround::ClearData()
{
	dGeomTriMeshDataBuildSimple(_mesh_data, nullptr, 0, nullptr, 0);
	_vertices.clear();
	_triangle_indices.clear();
	_normals.clear();
}

void MeshSurround::AddTriangle(Triangle3 const & triangle, Vector3 const & normal)
{
	ASSERT(NearEqual(geom::Length(normal), 1.f, .001f));

	TriangleIndices indices;
	indices[0] = _vertices.size();
	indices[1] = indices[0] + 2;
	indices[2] = indices[0] + 1;
	_triangle_indices.push_back(indices);

	for (const auto & vertex : triangle.points)
	{
		_vertices.push_back(vertex);
	}

	_normals.push_back(- normal);

#if defined(DEBUG_TRIANGLES)
	gfx::Debug::Color random_color(Random::sequence.GetUnit<float>(), Random::sequence.GetUnit<float>(), Random::sequence.GetUnit<float>(), .5f);
	gfx::Debug::AddTriangle(triangle, random_color);
	auto average = geom::Center(triangle);
	gfx::Debug::AddLine(average, average + normal, gfx::Debug::Color::White());
#endif
}

bool MeshSurround::IsEmpty() const
{
	ASSERT(_triangle_indices.empty() == _vertices.empty());
	ASSERT(_triangle_indices.empty() == _normals.empty());
	return _triangle_indices.empty();
}

void MeshSurround::RefreshData()
{
	dGeomTriMeshDataBuildSingle1(_mesh_data,
		_vertices.data(), sizeof(VerticesVector::value_type), _vertices.size(), 
		_triangle_indices.data(), _triangle_indices.size() * 3, sizeof(TriangleIndicesVector::value_type), 
		_normals.data());
}
