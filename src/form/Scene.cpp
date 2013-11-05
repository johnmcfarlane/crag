//
// form/Scene.cpp
// Crag
//
// Created by John on 2/8/10.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "Scene.h"

#include "Mesh.h"

#include "form/Surrounding.h"
#include "form/Polyhedron.h"

using namespace form;

/////////////////////////////////////////////////////////////////
// Scene

Scene::Scene(size_t min_num_quaterne, size_t max_num_quaterne)
: _surrounding(ref(new Surrounding(max_num_quaterne)))
{
	_surrounding.SetNumQuaternaUsedTarget(int(min_num_quaterne));
}

Scene::~Scene()
{
	Clear();
	delete & _surrounding;
}

#if defined(VERIFY)
void Scene::Verify() const
{
	if (! _surrounding.GetPoints().IsEmpty())
	{
		VerifyTrue(! formation_map.empty());
	}

	VerifyObject(_surrounding);
}

/*void SceneVerifyTrue(Mesh const & m) const
{
	VertexBuffer const & vb = * m.GetPoints();
	IndexBuffer const & ib = m.GetIndices();
	
	unsigned int const * indices = ib.GetArray();
	unsigned int const * const indices_end = indices + ib.GetSize();
	for (unsigned int const * it = indices; it != indices_end; ++ it) {
		Vertex const & v = vb[* it];
		Verify(v);
	}
}*/
#endif

void Scene::Clear()
{
	for (FormationMap::iterator it = formation_map.begin(); it != formation_map.end(); ++ it) {
		FormationPair & pair = * it;
		DeinitPolyhedron(pair);
	}
	
	formation_map.clear();
}

Surrounding & Scene::GetSurrounding()
{
	return _surrounding;
}

Surrounding const & Scene::GetSurrounding() const
{
	return _surrounding;
}

// Change the local co-ordinate system so that 0,0,0 in local space is o in global space.
void Scene::OnOriginReset(geom::abs::Vector3 const & origin) 
{
	// The difficult bit: fix all our data which relied on the old origin.
	ResetFormations(origin);
}

void Scene::AddFormation(Formation & formation, geom::abs::Vector3 const & origin)
{
	ASSERT(formation_map.find(& formation) == formation_map.end());
	FormationMap::iterator i = formation_map.insert(formation_map.begin(), FormationPair(& formation, Polyhedron(formation)));
	InitPolyhedron(* i, origin);
}

void Scene::RemoveFormation(Formation const & formation)
{
	FormationMap::iterator i = formation_map.find(& formation);
	ASSERT(i != formation_map.end());
	
	DeinitPolyhedron(* i);
	formation_map.erase(i);
}

Polyhedron const * Scene::GetPolyhedron(Formation const & formation) const
{
	FormationMap::const_iterator found = formation_map.find(& formation);
	if (found == formation_map.end())
	{
		return nullptr;
	}
	else
	{
		return & found->second;
	}
}

void Scene::Tick(geom::rel::Ray3 const & camera_ray)
{
	_surrounding.Tick(camera_ray);
	TickModels();
}

void Scene::GenerateMesh(Mesh & mesh, geom::abs::Vector3 const & origin) const
{
	_surrounding.GenerateMesh(mesh);
	
	MeshProperties & properties = mesh.GetProperties();
	properties._origin = origin;
	properties._num_quaterne = _surrounding.GetNumQuaternaUsed();
}

// Currently just updates the formation_map contents.
void Scene::TickModels()
{
	for (auto& pair : formation_map)
	{
		TickPolyhedron(pair.second);
	}
}

void Scene::ResetPolyhedronOrigins(geom::abs::Vector3 const & origin)
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		Polyhedron & polyhedron = pair.second;
		polyhedron.SetOrigin(origin);
	}
}

void Scene::ResetFormations(geom::abs::Vector3 const & origin)
{
	for (auto & pair : formation_map) 
	{
		DeinitPolyhedron(pair);
	}
	
	_surrounding.OnReset();

	for (auto & pair : formation_map) 
	{
		InitPolyhedron(pair, origin);
	}
}

void Scene::TickPolyhedron(Polyhedron & polyhedron)
{
	RootNode & root_node = polyhedron._root_node;
	
	if (root_node.IsExpandable()) 
	{
		VerifyObject(* this);
		_surrounding.ExpandNode(root_node);
		VerifyObject(* this);
	}
}

// The given pair contains a formation and a polyhedron.
void Scene::InitPolyhedron(FormationPair & pair, geom::abs::Vector3 const & origin)
{
	Polyhedron & polyhedron = pair.second;

	PointBuffer & points = _surrounding.GetPoints();
	
	polyhedron.Init(origin, points);
}

void Scene::DeinitPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	// Collapse the root node by fair means or foul.
	RootNode & root_node = polyhedron._root_node;

	_surrounding.CollapseNodes(root_node);
	ASSERT(! root_node.HasChildren());

	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(_surrounding.GetPoints());
}
