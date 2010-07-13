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
: node_buffer(1024)
, observer_pos(Vector3f::Zero())
, relative_observer_pos(Vector3f::Zero())
, origin(Vector3f::Zero())
{
}

form::Scene::~Scene()
{
	Clear();

	for (FormationMap::iterator it = formation_map.begin(); it != formation_map.end(); ++ it) {
		FormationPair & pair = * it;
		DeinitModel(pair);
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
		DeinitModel(pair);
	}
}

sim::Vector3 const & form::Scene::GetObserverPos() const
{
	return observer_pos;
}

void form::Scene::SetObserverPos(sim::Vector3 const & pos, sim::Vector3 const & dir) 
{
	observer_pos = pos;
	relative_observer_pos = observer_pos - origin;
	observer_dir = dir;
}

sim::Vector3 const & form::Scene::GetOrigin() const
{
	return origin;
}

void form::Scene::SetOrigin(sim::Vector3 const & o) 
{
	if (o != origin) 
	{
		origin = o;
		SetObserverPos(observer_pos, observer_dir);

		ResetFormations();
	}
}

void form::Scene::Tick(FormationSet const & formation_set)
{
	node_buffer.Tick(relative_observer_pos, observer_dir);
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
		Polyhedron const & model = pair.second;
		f(formation, model);
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

form::Polyhedron const & form::Scene::GetModel(Formation const & formation) const
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
				TickModel(pair->second);
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
			InitModel(* pair);
			TickModel(pair->second);
			++ m;
			++ s;
		}
		else {
			// removed
			DeinitModel(* m);
			formation_map.erase(m ++);
		}
	}
}

void form::Scene::ResetFormations()
{
	node_buffer.LockTree();

	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) {
		FormationPair & pair = * i;
		DeinitModel(pair);
	}
	
	node_buffer.OnReset();

	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) {
		FormationPair & pair = * i;
		InitModel(pair);
	}

	node_buffer.UnlockTree();
}

void form::Scene::TickModel(Polyhedron & model)
{
	form::RootNode & root_node = model.root_node;
	
	if (root_node.IsExpandable()) 
	{
		VerifyObject(* this);		
		node_buffer.ExpandNode(root_node);
		VerifyObject(* this);
	}
}

void form::Scene::InitModel(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Polyhedron & model = pair.second;
	
	Shader * shader = formation.shader_factory.Create(formation);
	shader->SetOrigin(origin);
	model.SetShader(shader);

	model.root_node.Init(formation.seed, node_buffer.GetPoints());
	model.root_node.SetCenter(formation.position - origin, formation.scale);
}

void form::Scene::DeinitModel(FormationPair & pair)
{
	Polyhedron & model = pair.second;
	
	RootNode & root_node = model.root_node;
#if ! defined(FAST_SCENE_RESET)
	node_buffer.CollapseNode(root_node);
#else
	root_node.children = nullptr;
#endif
	root_node.Deinit(node_buffer.GetPoints());
	
	delete & model.GetShader();
	model.SetShader(nullptr);
}

void form::Scene::ResetModel(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Polyhedron & model = pair.second;
	
	model.GetShader().SetOrigin(origin);
	
	RootNode & root_node = model.root_node;
	node_buffer.CollapseNode(root_node);
	model.root_node.SetCenter(formation.position - origin, formation.scale);
}

