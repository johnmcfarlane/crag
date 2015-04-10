//
//  ForEachNodeFace.h
//  crag
//
//  Created by John on 6/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"
#include "Node.h"
#include "Point.h"

#include "geom/Intersection.h"


namespace form
{
	
	// Helper function for ForEachNodeFace function.
	inline Node::Triplet const * TripletMod(Node::Triplet const * t, Node::Triplet const * last)
	{
		assert(t >= last - 2);
		assert(t < last + 4);
		return (t <= last) ? t : t - 3;
	}
	
	
	// Helper function for ForEachNodeFace function;
	// Calculates normal for given face and passes it on to functor.
	template<typename FaceFunctor> void AddFace(Point & a, Point & b, Point & c, float score, FaceFunctor & f)
	{
		geom::Vector3f normal = geom::UnitNormal(Triangle3(a.pos, b.pos, c.pos));
		f (a, b, c, normal, score);
	}
	
	
	// Given a Node, node, calculates its triangles and passes them to functor, f, via
	// member function AddFace(a, b, c, norm) where a, b and c are the points of a triangle 
	// and norm is the triangle's normal.
	template<typename FaceFunctor> void ForEachNodeFace(Node const & node, FaceFunctor f)
	{
		// Generally, it's advised to call this fn only for leaf nodes. 
		// But not required.
		
		// Step 1: Determine the number of mid-points and note missing / solitary mid-points.
		Node::Triplet const * odd_one_out[2] = { nullptr, nullptr };
		int num_mid_points = 0;
		
		Node::Triplet const * triple = node.triple;
		Node::Triplet const * t_last = triple + 2;
		for (Node::Triplet const * t_it = triple; ; ++ t_it)
		{
			if (t_it->cousin == nullptr || t_it->mid_point == nullptr)
			{
				odd_one_out[false] = t_it;
			}
			else 
			{
				odd_one_out[true] = t_it;
				++ num_mid_points;
			}
			
			if (t_it == t_last)
			{
				break;
			}
		}
		
		// Step 2: Given the number of mid-points, construct the faces possible from mid-points and corners. 
		switch (num_mid_points)
		{
			case 0: 
			{
				assert(odd_one_out[0] != nullptr);
				assert(odd_one_out[1] == nullptr);

				// Only the corners are available. Has the advantage that we know what the normal is.
				f (* triple[0].corner, * triple[1].corner, * triple[2].corner, node.normal, node.score);
			}	break;
				
			case 1: 
			{
				assert(odd_one_out[0] != nullptr);
				assert(odd_one_out[1] != nullptr);
				
				// A single mid-point means that the triangle can be divided in two.
				Node::Triplet const * midpoint_triplet_0 = odd_one_out[true];
				Point * midpoint_0 = midpoint_triplet_0->mid_point;
				Point * corner_0 = midpoint_triplet_0->corner;

				AddFace(* midpoint_0, * corner_0, * TripletMod(midpoint_triplet_0 + 1, t_last)->corner, node.score, f);
				AddFace(* corner_0, * midpoint_0, * TripletMod(midpoint_triplet_0 + 2, t_last)->corner, node.score, f);
			}	break;
				
			case 2: 
			{
				assert(odd_one_out[0] != nullptr);
				assert(odd_one_out[1] != nullptr);
				
				// Two mid-points means a triangle between them and their common corner
				// and a quad made from the rest of the node. 
				Node::Triplet const * midpoint_triplet_0 = odd_one_out[false];
				Node::Triplet const * midpoint_triplet_1 = TripletMod(midpoint_triplet_0 + 1, t_last);
				Node::Triplet const * midpoint_triplet_2 = TripletMod(midpoint_triplet_0 + 2, t_last);
				
				Point * midpoint_1 = midpoint_triplet_1->mid_point;
				Point * midpoint_2 = midpoint_triplet_2->mid_point;
				
				// Generate good triangle.
				AddFace(* midpoint_triplet_0->corner, * midpoint_2, * midpoint_1, node.score, f);
				
				// Generate wonky quad.
				AddFace(* midpoint_triplet_1->corner, * midpoint_triplet_2->corner, * midpoint_triplet_2->mid_point, node.score, f);
				AddFace(* midpoint_triplet_1->mid_point, * midpoint_triplet_2->mid_point, * midpoint_triplet_2->corner, node.score, f);
			}	break;
				
			case 3: 
			{
				assert(odd_one_out[0] == nullptr);
				assert(odd_one_out[1] != nullptr);
				
				AddFace(* triple[0].corner, * triple[2].mid_point, * triple[1].mid_point, node.score, f);
				AddFace(* triple[1].corner, * triple[0].mid_point, * triple[2].mid_point, node.score, f);
				AddFace(* triple[2].corner, * triple[1].mid_point, * triple[0].mid_point, node.score, f);
				AddFace(* triple[0].mid_point, * triple[1].mid_point, * triple[2].mid_point, node.score, f);
			}	break;
		}
	}

}
