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

#include "form/Formation.h"
#include "form/PointBuffer.h"
#include "form/Shader.h"

#include "core/memory.h"

using namespace form;

namespace
{
	void InitRootNode(Node & root_node, int init_seed, Point * root_points[4])
	{
		Point * corner = root_points[0];

		root_node.triple[0].corner = corner;
		root_node.triple[1].corner = corner;
		root_node.triple[2].corner = corner;
		
		root_node.center = geom::Vector3f::Zero();
		root_node.area = 0;
		root_node.normal = geom::Vector3f::Zero();
		root_node.score = std::numeric_limits<float>::max();
	
		root_node.seed = init_seed;
	
		for (int i = 0; i < 3; ++ i)
		{
			Node::Triplet & t = root_node.triple[i];
			t.mid_point = root_points[i + 1];
			t.cousin = & root_node;
		}
	
		// At this point, the four verts still need setting up.
	}

	void DeinitRootNode(Node & root_node, PointBuffer & points)
	{
		ASSERT(! root_node.HasChildren());
	
		points.Destroy(root_node.triple[0].corner);
	
		for (int i = 0; i < 3; ++ i) {
			root_node.triple[i].corner = nullptr;
		
			points.Destroy(root_node.triple[i].mid_point);
			root_node.triple[i].mid_point = nullptr;
		
			root_node.triple[i].cousin = nullptr;
		}
	}

	void GetRootNodePoints(Node const & root_node, Point * points[4])
	{
		// TODO: Verify would include check that {points[0] == (triple[1]} == triple[2])
		points[0] = root_node.triple[0].corner;
		points[1] = root_node.triple[0].mid_point;
		points[2] = root_node.triple[1].mid_point;
		points[3] = root_node.triple[2].mid_point;
	}
}

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
	_root_node.SetPolyhedron(this);
	
	CRAG_VERIFY(* this);
}

Polyhedron::Polyhedron(Polyhedron const & rhs)
: _shape(rhs._shape)
, _formation(rhs._formation)
{
	_root_node.SetPolyhedron(this);

	CRAG_VERIFY(* this);
}

Polyhedron::~Polyhedron()
{
	CRAG_VERIFY(* this);
	
	_root_node.SetPolyhedron(nullptr);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Polyhedron, self)
	CRAG_VERIFY(self._root_node);
	CRAG_VERIFY_EQUAL(self._root_node.GetPolyhedron(), & self);
CRAG_VERIFY_INVARIANTS_DEFINE_END

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
	InitRootNode(_root_node, _formation.GetSeed(), root_points);
}

void Polyhedron::Deinit(PointBuffer & point_buffer)
{
	DeinitRootNode(_root_node, point_buffer);
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

Node const & Polyhedron::GetRootNode() const
{
	return _root_node;
}

void Polyhedron::SetOrigin(geom::abs::Vector3 const & origin)
{
	geom::abs::Sphere3 const & shape = _formation.GetShape();
	_shape.center = geom::AbsToRel<double>(shape.center, origin);
	_shape.radius = shape.radius;
	
	Point * root_points[4];
	GetRootNodePoints(_root_node, root_points);

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
