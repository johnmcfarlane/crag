/*
 *  Polyhedron.cpp
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Polyhedron.h"
#include "Shader.h"
#include "VertexBuffer.h"

#include "core/memory.h"


////////////////////////////////////////////////////////////////////////////////
// Node accessors

form::Polyhedron & form::GetModel(Node & node)
{
	RootNode & root_node = GetRootNode(node);
	
	Polyhedron & owner = root_node.GetOwner();
	Assert(& owner.root_node == & root_node);
	
	return owner;
}


/////////////////////////////////////////////////////////////////
// form::Polyhedron

form::Polyhedron::Polyhedron()
: shader(nullptr)
{
	root_node = RootNode(* this);
}

form::Polyhedron::Polyhedron(Polyhedron const & rhs)
: shader(nullptr)
{
	root_node = RootNode(* this);
	
	// Can only copy default-state objects.
	Assert(rhs.shader == nullptr);
}

form::Polyhedron::~Polyhedron()
{
	delete shader;
}

void form::Polyhedron::SetShader(form::Shader * init_shader)
{
	// Currently, no need ever to change shader.
	Assert((shader == nullptr) != (init_shader == nullptr));
	
	shader = init_shader;
}

form::Shader & form::Polyhedron::GetShader()
{
	Assert(shader != nullptr);
	return * shader;
}

