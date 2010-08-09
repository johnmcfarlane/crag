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
#include "VertexBuffer.h"

#include "form/Formation.h"
#include "form/node/PointBuffer.h"
#include "form/node/Shader.h"

#include "core/memory.h"


////////////////////////////////////////////////////////////////////////////////
// Node accessors

form::Polyhedron & form::GetPolyhedron(Node & node)
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

form::Shader & form::Polyhedron::GetShader()
{
	Assert(shader != nullptr);
	return * shader;
}

void form::Polyhedron::Init(Formation const & formation, sim::Vector3 const & origin, PointBuffer & point_buffer)
{
	// Initialize the shader.
	shader = formation.shader_factory.Create(formation);
	shader->SetOrigin(origin);
	
	// Create me some points.
	// (These are the four points of the initial tetrahedron.
	// TODO: Use the word tetrahedron loads more. It sounds clever yet is easy to wikipedarize. 
	// TODO: GetPoints should be the only way we grab points for the benefit of shader.
	Point * root_points[4] = 
	{
		point_buffer.Alloc(),
		point_buffer.Alloc(),
		point_buffer.Alloc(),
		point_buffer.Alloc()
	};
	
	// Initialize the points using the shader.
	shader->InitRootPoints(root_points);	

	// Initialize the root node with the points
	root_node.Init(formation.seed, root_points);
}

void form::Polyhedron::Deinit(PointBuffer & point_buffer)
{
	root_node.Deinit(point_buffer);
	
	delete shader;
	shader = nullptr;
}

void form::Polyhedron::SetOrigin(sim::Vector3 const & origin, PointBuffer & point_buffer)
{
	shader->SetOrigin(origin);
	
	Point * root_points[4];
	root_node.GetPoints(root_points);
	shader->InitRootPoints(root_points);
	
#if defined(SUPERFAST_SCENE_RESET)	
	for (int depth = 0; ResetOrigin(root_node, point_buffer, depth); ++ depth)
	{
	}
#endif
}

bool form::Polyhedron::ResetOrigin(Node & node, PointBuffer & point_buffer, int depth)
{
	Node * children = node.children;
	if (children == nullptr)
	{
		return false;
	}
	
	if (depth == 0)
	{
		children[0].Reinit(* shader, point_buffer);
		children[1].Reinit(* shader, point_buffer);
		children[2].Reinit(* shader, point_buffer);
		children[3].Reinit(* shader, point_buffer);
		return true;
	}
	else 
	{
		-- depth;
		return	ResetOrigin(children[0], point_buffer, depth)
			|	ResetOrigin(children[1], point_buffer, depth)
			|	ResetOrigin(children[2], point_buffer, depth)
			|	ResetOrigin(children[3], point_buffer, depth);
	}
}
