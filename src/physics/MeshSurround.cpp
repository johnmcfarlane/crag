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
, _triangles(std::move(rhs._triangles))
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
	_triangles.clear();
	_normals.clear();
}

void MeshSurround::AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, Vector3 const & normal)
{
	ASSERT(NearEqual(geom::Length(normal), 1.f, .001f));

	Triangle triangle;
	triangle[0] = _vertices.size();
	triangle[1] = triangle[0] + 2;
	triangle[2] = triangle[0] + 1;
	_triangles.push_back(triangle);

	_vertices.push_back(a);
	_vertices.push_back(b);
	_vertices.push_back(c);

	_normals.push_back(- normal);

#if defined(DEBUG_TRIANGLES)
	gfx::Debug::Color random_color(Random::sequence.GetUnit<float>(), Random::sequence.GetUnit<float>(), Random::sequence.GetUnit<float>(), .5f);
	gfx::Debug::AddTriangle(a, b, c, random_color);
	auto average = (a + b + c) / 3.f;
	gfx::Debug::AddLine(average, average + normal, gfx::Debug::Color::White());
#endif
}

void MeshSurround::RefreshData()
{
	dGeomTriMeshDataBuildSingle1(_mesh_data,
		_vertices.data(), sizeof(VerticesVector::value_type), _vertices.size(), 
		_triangles.data(), _triangles.size() * 3, sizeof(TrianglesVector::value_type), 
		_normals.data());
}
