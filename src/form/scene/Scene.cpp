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

#include "form/Formation.h"
#include "form/FormationManager.h"

#include "form/node/NodeBuffer.h"
#include "form/node/Shader.h"

#include "form/scene/Polyhedron.h"

#include "core/ConfigEntry.h"


namespace
{
	CONFIG_DEFINE (max_observer_position_length, sim::Scalar, 2500);
}


/////////////////////////////////////////////////////////////////
// form::Scene

form::Scene::Scene()
: node_buffer(new NodeBuffer)
, camera_ray(sim::Ray3::Zero())
, camera_ray_relative(sim::Ray3::Zero())
, origin(sim::Vector3::Zero())
{
}

form::Scene::~Scene()
{
	Clear();
	delete node_buffer;
}

#if defined(VERIFY)
void form::Scene::Verify() const
{
	VerifyObject(* node_buffer);
	
	//	int count_num_nodes = CountChildNodes();
	//	int buffer_num_nodes = node_buffer.GetNumNodes();
	
	//	Assert(count_num_nodes == buffer_num_nodes);
	
	//	node_buffer.Verify();
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

#if defined(DUMP)
DUMP_OPERATOR_DEFINITION(form, Scene)
{
	DumpStream indented = lhs;

	for (form::Scene::FormationMap::const_iterator it = rhs.formation_map.begin(); it != rhs.formation_map.end(); ++ it) {
		form::Scene::FormationPair const & pair = * it;
		lhs << lhs.NewLine() << "root_node:";
		indented << pair.second.root_node;
	}

	lhs << lhs.NewLine() << "node_buffer:";
	indented << rhs.node_buffer;
	
	return lhs;
}
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
	return ref(node_buffer);
}

form::NodeBuffer const & form::Scene::GetNodeBuffer() const
{
	return ref(node_buffer);
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

bool form::Scene::IsOriginOk() const
{
	// The real test: Is the observer not far enough away from the 
	// current origin that visible inaccuracies might become apparent?
	double observer_position_length = Length(camera_ray_relative.position);
	return observer_position_length < max_observer_position_length;
}

void form::Scene::AddFormation(Formation const & formation)
{
	Assert(formation_map.find(& formation) == formation_map.end());
	FormationMap::iterator i = formation_map.insert(formation_map.begin(), FormationPair(& formation, Polyhedron()));
	InitPolyhedron(* i);
}

void form::Scene::RemoveFormation(Formation const & formation)
{
	FormationMap::iterator i = formation_map.find(& formation);
	Assert(i != formation_map.end());
	
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
	form::Ray3 form_camera_ray(camera_ray_relative.position, camera_ray_relative.direction);
	node_buffer->Tick(form_camera_ray);
	TickModels();
}

void form::Scene::GenerateMesh(Mesh & mesh) const
{
	node_buffer->GenerateMesh(mesh);
	
	MeshProperties & properties = mesh.GetProperties();
	properties._origin = origin;
	properties._num_quaterne = node_buffer->GetNumQuaternaUsed();
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
		polyhedron.SetOrigin(origin, node_buffer->GetPoints());
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
	
	node_buffer->OnReset();

	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		InitPolyhedron(pair);
	}
}

void form::Scene::TickPolyhedron(Polyhedron & polyhedron)
{
	form::RootNode & root_node = polyhedron.root_node;
	
	if (root_node.IsExpandable()) 
	{
		VerifyObject(* this);
		node_buffer->ExpandNode(root_node);
		VerifyObject(* this);
	}
}

// The given pair contains a formation and a polyhedron.
void form::Scene::InitPolyhedron(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Polyhedron & polyhedron = pair.second;

	PointBuffer & points = node_buffer->GetPoints();
	
	polyhedron.Init(formation, origin, points);
}

void form::Scene::DeinitPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	// Collapse the root node by fair means or foul.
	RootNode & root_node = polyhedron.root_node;
	
	node_buffer->CollapseNodes(root_node);
	
	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(node_buffer->GetPoints());
}