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

Scene::Scene(int min_num_quaterne, int max_num_quaterne)
: _surrounding(new Surrounding(max_num_quaterne))
{
	_surrounding->SetTargetNumQuaterna(min_num_quaterne);
}

Scene::~Scene()
{
	Clear();
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Scene, scene)
	auto const & surrounding = * scene._surrounding;
	if (! surrounding.GetPoints().IsEmpty())
	{
		CRAG_VERIFY_FALSE(scene.formation_map.empty());
	}

	CRAG_VERIFY(surrounding);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Scene::Clear()
{
	for (FormationMap::iterator it = formation_map.begin(); it != formation_map.end(); ++ it) {
		FormationPair & pair = * it;
		DeinitPolyhedron(pair);
	}
	
	formation_map.clear();
	
	_is_settled = false;
}

Surrounding & Scene::GetSurrounding()
{
	return * _surrounding;
}

Surrounding const & Scene::GetSurrounding() const
{
	return * _surrounding;
}

// Change the local co-ordinate system so that 0,0,0 in local space is o in global space.
void Scene::OnSpaceReset(geom::Space const & space, gfx::LodParameters const & lod_parameters)
{
	auto num_quaterna = _surrounding->GetNumQuaternaUsed();

	// The difficult bit: fix all our data which relied on the old space.
	ResetFormations(space);

	_surrounding->SetTargetNumQuaterna(num_quaterna);

	while (_surrounding->GetNumQuaternaUsed() < num_quaterna)
	{
		Tick(lod_parameters);
	}
}

bool Scene::IsSettled() const
{
	return _is_settled;
}

bool Scene::IsPaused() const
{
	return _is_paused;
}

void Scene::SetPaused(bool paused)
{
	_is_paused = paused;
}

void Scene::AddFormation(Formation & formation, geom::Space const & space)
{
	ASSERT(formation_map.find(& formation) == formation_map.end());
	FormationMap::iterator i = formation_map.insert(formation_map.begin(), FormationPair(& formation, Polyhedron(formation)));
	InitPolyhedron(* i, space);
	
	_is_settled = false;
}

void Scene::RemoveFormation(Formation const & formation)
{
	FormationMap::iterator i = formation_map.find(& formation);
	ASSERT(i != formation_map.end());
	
	DeinitPolyhedron(* i);
	formation_map.erase(i);
	
	_is_settled = false;
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

bool Scene::Tick(gfx::LodParameters const & lod_parameters)
{
	CRAG_VERIFY(lod_parameters);
	CRAG_VERIFY_FALSE(_is_paused);
	
	bool changed = _surrounding->Tick(lod_parameters);
	TickModels();
	
	if (! changed)
	{
		_is_settled = true;
	}
	
	return changed;
}

void Scene::GenerateMesh(Mesh & mesh, geom::Space const & space) const
{
	mesh.Clear();
	_surrounding->ResetMeshPointers();
	
	_surrounding->GenerateMesh(mesh);
	
	MeshProperties & properties = mesh.GetProperties();
	properties._space = space;
	properties._num_quaterne = _surrounding->GetNumQuaternaUsed();
}

// Currently just updates the formation_map contents.
void Scene::TickModels()
{
	for (auto& pair : formation_map)
	{
		TickPolyhedron(pair.second);
	}
}

void Scene::ResetPolyhedronSpaces(geom::Space const & space)
{
	for (FormationMap::iterator i = formation_map.begin(); i != formation_map.end(); ++ i) 
	{
		FormationPair & pair = * i;
		Polyhedron & polyhedron = pair.second;
		polyhedron.SetSpace(space);
	}
}

void Scene::ResetFormations(geom::Space const & space)
{
	for (auto & pair : formation_map) 
	{
		DeinitPolyhedron(pair);
	}
	
	_surrounding->OnReset();

	for (auto & pair : formation_map) 
	{
		InitPolyhedron(pair, space);
	}

	_is_settled = false;
}

void Scene::TickPolyhedron(Polyhedron & polyhedron)
{
	Node & root_node = polyhedron._root_node;
	
	if (root_node.IsExpandable()) 
	{
		CRAG_VERIFY(* this);
		_surrounding->ExpandNode(root_node);
		CRAG_VERIFY(* this);
	}
}

// The given pair contains a formation and a polyhedron.
void Scene::InitPolyhedron(FormationPair & pair, geom::Space const & space)
{
	Polyhedron & polyhedron = pair.second;

	PointBuffer & points = _surrounding->GetPoints();
	
	polyhedron.Init(space, points);
}

void Scene::DeinitPolyhedron(FormationPair & pair)
{
	Polyhedron & polyhedron = pair.second;
	
	// Collapse the root node by fair means or foul.
	Node & root_node = polyhedron._root_node;

	_surrounding->CollapseNodes(root_node);
	ASSERT(! root_node.HasChildren());

	// Continue deinitialization somewhere a bit calmer.
	polyhedron.Deinit(_surrounding->GetPoints());
}
