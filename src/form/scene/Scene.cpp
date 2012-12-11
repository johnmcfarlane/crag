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
: _node_buffer(ref(new NodeBuffer(min_num_quaterne, max_num_quaterne)))
, camera_ray(sim::Ray3::Zero())
, camera_ray_relative(sim::Ray3::Zero())
, origin(sim::Vector3::Zero())
{
	_node_buffer.SetNumQuaternaUsedTarget(min_num_quaterne);
}

form::Scene::~Scene()
{
	Clear();
	delete & _node_buffer;
}

#if defined(VERIFY)
void form::Scene::Verify() const
{
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

sim::Ray3 const & form::Scene::GetCameraRay() const
{
	return camera_ray;
}

void form::Scene::SetCameraRay(sim::Ray3 const & cr) 
{
	camera_ray = cr;
	camera_ray_relative = sim::Ray3(cr.position - origin, cr.direction);
}

sim::Vector3 const & form::Scene::GetOrigin() const
{
	return origin;
}

// Change the local co-ordinate system so that 0,0,0 in local space is o in global space.
void form::Scene::SetOrigin(sim::Vector3 const & o) 
{
	if (o != origin) 
	{
		origin = o;
		
		// Setting camera ray to itself cause the local camera ray to be recalculated.
		SetCameraRay(camera_ray);

		// The difficult bit: fix all our data which relied on the old origin.
		ResetFormations();
	}
}

void form::Scene::AddFormation(Formation & formation)
{
	ASSERT(formation_map.find(& formation) == formation_map.end());
	FormationMap::iterator i = formation_map.insert(formation_map.begin(), FormationPair(& formation, Polyhedron(formation)));
	InitPolyhedron(* i);
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

void form::Scene::Tick()
{
	form::Ray3 form_camera_ray(geom::Cast<Scalar>(camera_ray_relative));
	_node_buffer.Tick(form_camera_ray);
	TickModels();
}

void form::Scene::GenerateMesh(Mesh & mesh) const
{
	_node_buffer.GenerateMesh(mesh);
	
	MeshProperties & properties = mesh.GetProperties();
	properties._origin = origin;
	properties._num_quaterne = _node_buffer.GetNumQuaternaUsed();
}

// Currently just updates the formation_map contents.
void form::Scene::TickModels()
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i)
	{
		TickPolyhedron(i->second);
	}
}

void form::Scene::ResetPolyhedronOrigins()
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		Polyhedron & polyhedron = pair.second;
		polyhedron.SetOrigin(origin);
	}
}

// TODO: Write an ungraceful/fast version of this.
void form::Scene::ResetFormations()
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		DeinitPolyhedron(pair);
	}
	
	_node_buffer.OnReset();

	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		InitPolyhedron(pair);
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
void form::Scene::InitPolyhedron(FormationPair & pair)
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
	
	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(_node_buffer.GetPoints());
}
