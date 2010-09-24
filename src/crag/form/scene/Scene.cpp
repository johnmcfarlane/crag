/*
 *  form/Scene.cpp
 *  Crag
 *
 *  Created by John on 2/8/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Scene.h"

#include "Mesh.h"

#include "form/Formation.h"
#include "form/FormationFunctor.h"

#include "form/node/Shader.h"


/////////////////////////////////////////////////////////////////
// form::Scene

form::Scene::Scene()
: node_buffer()
, origin(sim::Vector3::Zero())
{
}

form::Scene::~Scene()
{
	Clear();

	for (FormationMap::iterator it = formation_map.begin(); it != formation_map.end(); ++ it) {
		FormationPair & pair = * it;
		DeinitPolyhedron(pair);
	}
}

#if VERIFY
void form::Scene::Verify() const
{
	VerifyObject(node_buffer);

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

#if DUMP
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
#if defined(MEGAFAST_SCENE_RESET)
		sim::Vector3 origin_delta = o - origin;
#endif
		
		node_buffer.LockTree();
		origin = o;
		
		// Setting camera ray to itself cause the local camera ray to be recalculated.
		SetCameraRay(camera_ray);

		// The difficult bit: fix all our data which relied on the old origin.
#if defined(SUPERFAST_SCENE_RESET)
		ResetPolyhedronOrigins();
#elif defined(MEGAFAST_SCENE_RESET)
		ResetPolyhedronOrigins();
		VerifyObject(* this);
		node_buffer.ResetNodeOrigins(origin_delta);
#else
		ResetFormations();
#endif

		node_buffer.UnlockTree();
	}
}

void form::Scene::Tick(FormationSet const & formation_set)
{
	node_buffer.Tick(camera_ray_relative);
	TickModels(formation_set);
}

void form::Scene::ForEachFormation(FormationFunctor & f) const
{
	f.SetSceneOrigin(origin);

	node_buffer.LockTree();
	
	for (form::Scene::FormationMap::const_iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		form::Scene::FormationPair const & pair = * i;
		Formation const & formation = * pair.first;
		Polyhedron const & polyhedron = pair.second;
		f(formation, polyhedron);
	}
	
	node_buffer.UnlockTree();
}

void form::Scene::GenerateMesh(Mesh & mesh) 
{
	node_buffer.GenerateMesh(mesh);
}

int form::Scene::GetNumQuaternaUsed() const
{
	return node_buffer.GetNumQuaternaUsed();
}

int form::Scene::GetNumQuaternaUsedTarget() const
{
	return node_buffer.GetNumQuaternaUsedTarget();
}

void form::Scene::SetNumQuaternaUsedTarget(int n)
{
	node_buffer.SetNumQuaternaUsedTarget(n);
}

form::Polyhedron const & form::Scene::GetPolyhedron(Formation const & formation) const
{
	//return formation_map[& formation];
	FormationMap::const_iterator i = formation_map.find (& formation);
	Assert(i != formation_map.end());
	return i->second;
}

// Currently just updates the formation_map contents.
void form::Scene::TickModels(FormationSet const & formation_set)
{
	FormationSet::iterator s = formation_set.begin();
	FormationMap::iterator m = formation_map.begin();
	
	// Deal with elements of one objet being absent from the other and vica versa.
	while (true) {
		Formation * formation = (s != formation_set.end()) ? * s : nullptr;
		FormationPair * pair = (m != formation_map.end()) ? & (* m) : nullptr;

		// Are we at neither the end of one sequence or other?
		if (formation != nullptr && pair != nullptr) {
			
			// Do we have a match?
			if (formation == pair->first) {
				TickPolyhedron(pair->second);
				++ s;
				++ m;
				continue;
			}
			else {
				// Is the formation missing from our internal store?
				if (formation < pair->first) {
					pair = nullptr;
				}
				else {
					formation = nullptr;
				}
			}
		}
		else if (formation == nullptr && pair == nullptr) {
			break;
		}
		
		// Here, we know that a formation has been added or removed from <formations>.
		Assert((formation == nullptr) != (pair == nullptr));
		
		if (pair == nullptr) {
			// added
			m = formation_map.insert(m, FormationPair(formation, Polyhedron()));
			pair = & (* m);
			InitPolyhedron(* pair);
			TickPolyhedron(pair->second);
			++ m;
			++ s;
		}
		else {
			// removed
			DeinitPolyhedron(* m);
			formation_map.erase(m ++);
		}
	}
}

void form::Scene::ResetPolyhedronOrigins()
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		Polyhedron & polyhedron = pair.second;
		polyhedron.SetOrigin(origin, node_buffer.GetPoints());
	}
}

void form::Scene::ResetFormations()
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		DeinitPolyhedron(pair);
	}
	
	node_buffer.OnReset();

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
		node_buffer.ExpandNode(root_node);
	}
}

// The given pair contains a formation and a polyhedron.
void form::Scene::InitPolyhedron(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Polyhedron & polyhedron = pair.second;

	PointBuffer & points = node_buffer.GetPoints();
	
	polyhedron.Init(formation, origin, points);
}

void form::Scene::DeinitPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	// Collapse the root node by fair means or foul.
	RootNode & root_node = polyhedron.root_node;
#if defined(FAST_SCENE_RESET)
	root_node.children = nullptr;
#else
	node_buffer.CollapseNode(root_node);
#endif
	
	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(node_buffer.GetPoints());
}

void form::Scene::ResetPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	polyhedron.GetShader().SetOrigin(origin);
	
	RootNode & root_node = polyhedron.root_node;
	node_buffer.CollapseNode(root_node);
	
	Point * points[4];
	root_node.GetPoints(points);
	polyhedron.shader->InitRootPoints(points);
}
