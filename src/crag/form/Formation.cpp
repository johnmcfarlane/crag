/*
 *  form/Formation.cpp
 *  Crag
 *
 *  Created by john on 6/2/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/Formation.h"

#include "form/NodeTraversal.h"
#include "form/Mesh.h"


#if defined(GFX_DEBUG)
#define DEBUG_NORMALS 0
#else
#define DEBUG_NORMALS 0
#endif


form::Formation::Formation(ShaderFactory const & init_shader_factory, float init_scale)
: shader_factory(init_shader_factory)
, scale(init_scale)
, seed(-1)
{
	//Clear();

	// TODO: Really belong here?
	//FormationManager::AddFormation(this);
	
	VerifyObject(* this);
}

form::Formation::~Formation()
{
	VerifyObject(* this);
	
	// TODO: Really belong here?
	//FormationManager::RemoveFormation(this);
}

void form::Formation::SetPosition(Vector3f const & init_position)
{
	position = init_position;
}

void form::Formation::GenerateCollisionMesh(form::Mesh & mesh, sim::Sphere3 const & sphere) const
{
/*	Assert(mesh.GetNumPolys() == 0);
	
	CollideSphereFunctor collide(mesh, sphere);
	NodeStub root_stub = default_root_stub;
	root_stub.node = const_cast<Node *>(& root_node);
	
	ForEachPoly(root_stub, collide);*/
}


void form::Formation::Clear()
{
	Assert(false);
	/*ZeroObject(corners);
	ZeroObject(root_node);*/
}

/*void form::Formation::DeinitNode(Node & node)
{
}*/

/*#if DUMP
void form::Formation::Dump() const
{
	for (IteratorConst it (& root_node); it; ++ it)
	{
		for (int n = it.GetDepth(); n >= 0; -- n)
		{
			putchar(' ');
		}
		Node const * node = * it;
		node->Dump();
	}
}
#endif*/

#if VERIFY
void form::Formation::Verify() const
{
	//VerifyNode(root_node);
	
	//int functor_count = CountNumNodes();
	//int recursive_count = CountNumNodes(& root_node);
	//Assert(functor_count == recursive_count);
}

/*void form::Formation::VerifyNode(Node const & r) const
{
	r.Verify();
	
	for (int i = 0; i < NUM_ROOT_NODES; ++ i)
	{
		Node const * child = r.GetChild(i);
		if (child != nullptr)
		{
			child->Verify();
		}
	}
}*/

/*int form::Formation::CountNumNodes(form::Node const * node) const
{
	if (node == nullptr)
	{
		return 0;
	}
	
	NodeStub const * child_stubs = node->GetChildStubs();
	return 1
		+ CountNumNodes(child_stubs[0].node)
		+ CountNumNodes(child_stubs[1].node)
		+ CountNumNodes(child_stubs[2].node)
		+ CountNumNodes(child_stubs[3].node);
}*/
#endif

