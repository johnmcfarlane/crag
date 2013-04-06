//
//  GenerateMeshFunctor.h
//  crag
//
//  Created by John on 3/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Node.h"

#include "form/scene/Mesh.h"
#include "ForEachNodeFace.h"


namespace form 
{
	// forward-declarations
	class Mesh;
	

	// This node functor is called on every used node in the NodeNuffer and
	// uses the vertices associated with those nodes to generate the given mesh.
	class GenerateMeshFunctor
	{
		OBJECT_NO_COPY (GenerateMeshFunctor);

	public:
		GenerateMeshFunctor(geom::Vector2f leaf_score_range, Mesh & mesh) 
		: _leaf_score_range(leaf_score_range)
		, _mesh(mesh) 
#if defined(NODE_SCORE_HEAT_MAP)
		, _inv_range(255.99f / (leaf_score_range.y - leaf_score_range.x))
#endif
		{ 
		}
		
		// NodeBuffer::ForEachNode functor
		void operator() (Node & node)
		{
			if (node.IsLeaf()) 
			{
				ForEachNodeFace(node, * this);
			}
		}
		
		// ForEachNodeFace functor
		void operator()(Point & a, Point & b, Point & c, geom::Vector3f const & normal, float score)
		{
			Color color;
#if defined(NODE_SCORE_HEAT_MAP)
			if (score < _leaf_score_range.x)
			{
				ASSERT(false);
				color = Color::Red();
			}
			else if (score > _leaf_score_range.y)
			{
				color = Color::Green();
			}
			else
			{
				int t = static_cast<int>(_inv_range * (score - _leaf_score_range.x));
				ASSERT(t >= 0 && t < 256);
				color.r = 255 - t;
				color.g = t;
				color.b = t;
				color.a = 255;
			}
#else
			color = Color::White();
#endif
			
			_mesh.AddFace(a, b, c, normal, color);
		}
		
	private:
		geom::Vector2f _leaf_score_range;
		Mesh & _mesh;
#if defined(NODE_SCORE_HEAT_MAP)
		float _inv_range;
#endif
	};

}
