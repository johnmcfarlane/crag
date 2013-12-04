//
//  Polyhedron.cpp
//  crag
//
//  Created by John on 2/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Polyhedron.h"
#include "VertexBuffer.h"

#include "form/Formation.h"
#include "form/PointBuffer.h"
#include "form/Shader.h"

#include "core/memory.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// Node accessors

Polyhedron * form::GetPolyhedron(Node & node)
{
	auto parent = node.GetParent();
	if (parent)
	{
		return GetPolyhedron(* parent);
	}
	
	return node.GetPolyhedron();
}

/////////////////////////////////////////////////////////////////
// Polyhedron

Polyhedron::Polyhedron(Formation & formation)
: _shape(formation.GetShape())
, _formation(formation)
{
	_root_node = RootNode(* this);
}

Polyhedron::Polyhedron(Polyhedron const & rhs)
: _shape(rhs._shape)
, _formation(rhs._formation)
{
	_root_node = RootNode(* this);
}

void Polyhedron::Init(geom::abs::Vector3 const & origin, PointBuffer & point_buffer)
{
	// Initialize the shader.
	_shape.center = geom::AbsToRel<double>(_formation.GetShape().center, origin);
	Shader const & shader = _formation.GetShader();
	
	// Create me some points.
	// (These are the four points of the initial tetrahedron.
	Point * root_points[4] = 
	{
		point_buffer.Create(),
		point_buffer.Create(),
		point_buffer.Create(),
		point_buffer.Create()
	};
	
	// Initialize the points using the shader.
	shader.InitRootPoints(* this, root_points);	

	// Initialize the root node with the points
	_root_node.Init(_formation.GetSeed(), root_points);
}

void Polyhedron::Deinit(PointBuffer & point_buffer)
{
	_root_node.Deinit(point_buffer);
}

geom::abs::Sphere3 const & Polyhedron::GetShape() const
{
	return _shape;
}

Formation & Polyhedron::GetFormation()
{
	return _formation;
}

Formation const & Polyhedron::GetFormation() const
{
	return _formation;
}

RootNode const & Polyhedron::GetRootNode() const
{
	return _root_node;
}

void Polyhedron::SetOrigin(geom::abs::Vector3 const & origin)
{
	geom::abs::Sphere3 const & shape = _formation.GetShape();
	_shape.center = geom::AbsToRel<double>(shape.center, origin);
	_shape.radius = shape.radius;
	
	Point * root_points[4];
	_root_node.GetPoints(root_points);

	Shader const & shader = _formation.GetShader();
	shader.InitRootPoints(* this, root_points);
}

bool Polyhedron::ResetOrigin(Node & node, PointBuffer & point_buffer, int depth)
{
	Node * children = node.GetChildren();
	if (children == nullptr)
	{
		return false;
	}
	
	if (depth == 0)
	{
		children[0].Reinit(* this, point_buffer);
		children[1].Reinit(* this, point_buffer);
		children[2].Reinit(* this, point_buffer);
		children[3].Reinit(* this, point_buffer);
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
