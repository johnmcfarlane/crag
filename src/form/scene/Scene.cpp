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

#include "form/node/NodeBuffer.h"

#include "form/scene/Polyhedron.h"


/////////////////////////////////////////////////////////////////
// form::Scene

form::Scene::Scene(size_t min_num_quaterne, size_t max_num_quaterne)
: _node_buffer(ref(new NodeBuffer(max_num_quaterne)))
{
	_node_buffer.SetNumQuaternaUsedTarget(int(min_num_quaterne));
}

form::Scene::~Scene()
{
	Clear();
	delete & _node_buffer;
}

#if defined(VERIFY)
void form::Scene::Verify() const
{
	if (! _node_buffer.GetPoints().IsEmpty())
	{
		VerifyTrue(! formation_map.empty());
	}

	VerifyObject(_node_buffer);
}

/*void form::SceneVerifyTrue(Mesh const & m) const
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

void form::Scene::Clear()
{
	for (FormationMap::iterator it = formation_map.begin(); it != formation_map.end(); ++ it) {
		FormationPair & pair = * it;
		DeinitPolyhedron(pair);
	}
	
	formation_map.clear();
}

form::NodeBuffer & form::Scene::GetNodeBuffer()
{
	return _node_buffer;
}

form::NodeBuffer const & form::Scene::GetNodeBuffer() const
{
	return _node_buffer;
}

// Change the local co-ordinate system so that 0,0,0 in local space is o in global space.
void form::Scene::OnOriginReset(geom::abs::Vector3 const & origin) 
{
	// The difficult bit: fix all our data which relied on the old origin.
	ResetFormations(origin);
}

void form::Scene::AddFormation(Formation & formation, geom::abs::Vector3 const & origin)
{
	ASSERT(formation_map.find(& formation) == formation_map.end());
	FormationMap::iterator i = formation_map.insert(formation_map.begin(), FormationPair(& formation, Polyhedron(formation)));
	InitPolyhedron(* i, origin);
}

void form::Scene::RemoveFormation(Formation const & formation)
{
	FormationMap::iterator i = formation_map.find(& formation);
	ASSERT(i != formation_map.end());
	
	DeinitPolyhedron(* i);
	formation_map.erase(i);
}

form::Polyhedron const * form::Scene::GetPolyhedron(Formation const & formation) const
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

void form::Scene::Tick(geom::rel::Ray3 const & camera_ray)
{
	_node_buffer.Tick(camera_ray);
	TickModels();
}

void form::Scene::GenerateMesh(Mesh & mesh, geom::abs::Vector3 const & origin) const
{
	_node_buffer.GenerateMesh(mesh);
	
	MeshProperties & properties = mesh.GetProperties();
	properties._origin = origin;
	properties._num_quaterne = _node_buffer.GetNumQuaternaUsed();
}

// Currently just updates the formation_map contents.
void form::Scene::TickModels()
{
	for (auto& pair : formation_map)
	{
		TickPolyhedron(pair.second);
	}
}

void form::Scene::ResetPolyhedronOrigins(geom::abs::Vector3 const & origin)
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		Polyhedron & polyhedron = pair.second;
		polyhedron.SetOrigin(origin);
	}
}

void form::Scene::ResetFormations(geom::abs::Vector3 const & origin)
{
	for (auto & pair : formation_map) 
	{
		DeinitPolyhedron(pair);
	}
	
	_node_buffer.OnReset();

	for (auto & pair : formation_map) 
	{
		InitPolyhedron(pair, origin);
	}
}

void form::Scene::TickPolyhedron(Polyhedron & polyhedron)
{
	form::RootNode & root_node = polyhedron._root_node;
	
	if (root_node.IsExpandable()) 
	{
		VerifyObject(* this);
		_node_buffer.ExpandNode(root_node);
		VerifyObject(* this);
	}
}

// The given pair contains a formation and a polyhedron.
void form::Scene::InitPolyhedron(FormationPair & pair, geom::abs::Vector3 const & origin)
{
	Polyhedron & polyhedron = pair.second;

	PointBuffer & points = _node_buffer.GetPoints();
	
	polyhedron.Init(origin, points);
}

void form::Scene::DeinitPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	// Collapse the root node by fair means or foul.
	RootNode & root_node = polyhedron._root_node;

	_node_buffer.CollapseNodes(root_node);
	ASSERT(! root_node.HasChildren());

	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(_node_buffer.GetPoints());
}
