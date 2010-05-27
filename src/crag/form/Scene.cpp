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

#include "Formation.h"
//#include "NodeTraversal.h"
#include "Mesh.h"
#include "Shader.h"


/////////////////////////////////////////////////////////////////
// form::Scene

// the root node isn't a proper triangle
/*form::Node const form::Scene::default_root_stub = 
{
	nullptr,
	Vector3f::Zero(),
	std::numeric_limits<float>::max(),
	Vector3f::Zero(),
};*/


form::Scene::Scene()
: observer_pos(Vector3f::Zero())
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
	VertexBuffer const & vb = * m.GetVertices();
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

void form::Scene::SetObserverPos(sim::Vector3 const & o) 
{
	observer_pos = o;
	relative_observer_pos = observer_pos - origin;
}

sim::Vector3 const & form::Scene::GetOrigin() const
{
	return origin;
}

void form::Scene::SetOrigin(sim::Vector3 const & o) 
{
	VerifyObject(* this);

	if (o != origin) {
		origin = o;
		SetObserverPos(observer_pos);

		ResetFormations();
	}

	VerifyObject(* this);
}

void form::Scene::Tick(FormationSet const & formation_set)
{
	node_buffer.Tick(relative_observer_pos);
	TickModels(formation_set);
}

void form::Scene::GenerateMesh(Mesh & mesh) 
{
	node_buffer.GenerateMesh(mesh);
}

int form::Scene::GetNumQuaternaAvailable() const
{
	return node_buffer.GetNumQuaternaAvailable();
}

void form::Scene::SetNumQuaternaAvailable(int n)
{
	node_buffer.SetNumQuaternaAvailable(n);
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
			m = formation_map.insert(m, FormationPair(formation, Model()));
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
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) {
		FormationPair & pair = * i;
		ResetModel(pair);
	}
	
	node_buffer.OnReset();
}

void form::Scene::TickModel(Model & model) // TODO: Write a scene::forEach
{
	form::RootNode & root_node = model.root_node;
	
	if (root_node.IsExpandable()) {
		node_buffer.ExpandNode(root_node);
	}
}

void form::Scene::InitModel(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Model & model = pair.second;
	
	Shader * shader = formation.shader_factory.Create(formation);
	shader->SetOrigin(origin);
	model.SetShader(shader);

	model.root_node.Init(formation.seed, node_buffer.GetVertices());
	model.root_node.SetCenter(formation.position - origin, formation.scale);
}

void form::Scene::DeinitModel(FormationPair & pair)
{
	Model & model = pair.second;
	
	RootNode & root_node = model.root_node;
	node_buffer.CollapseNode(root_node);
	root_node.Deinit(node_buffer.GetVertices());
	
	delete & model.GetShader();
	model.SetShader(nullptr);
}

void form::Scene::ResetModel(FormationPair & pair)
{
	Formation const & formation = ref(pair.first);
	Model & model = pair.second;
	
	model.GetShader().SetOrigin(origin);
	
	RootNode & root_node = model.root_node;
	node_buffer.CollapseNode(root_node);
	model.root_node.SetCenter(formation.position - origin, formation.scale);
}

