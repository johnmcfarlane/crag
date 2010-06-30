/*
 *  ForEachNodeFace.h
 *  crag
 *
 *  Created by John on 6/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Node.h"


namespace form
{
	// Helper function for ForEachNodeFace function;
	// Calculates normal for given face and passes it on to functor.
	template<typename FaceFunctor> void AddFace(Point & a, Point & b, Point & c, FaceFunctor & f)
	{
		Vector3f normal = TriangleNormal(static_cast<Vector3 const &>(a), 
										 static_cast<Vector3 const &>(b), 
										 static_cast<Vector3 const &>(c));
		f.AddFace(a, b, c, FastNormalize(normal));
	}
	
	// Given a Node, n, calculates its triangles and passes them to functor, f, via
	// member function AddFace(a, b, c, n) where a, b and c are the points of a triangle 
	// and n is the triangle's normal.
	template<typename FaceFunctor> void ForEachNodeFace(Node const & n, FaceFunctor & f)
	{
		if (n.children != nullptr)
		{
			// Node, n, isn't a leaf node; its descendents are the ones with the faces.
			return;
		}
		
		// Step 1: Determine the number of mid-points and note missing / single mid-points.
		int triplet_indices[2] = { -1, -1 };	// NULL/non-NULL
		int num_mid_points = 0;
		for (int i = 0; i < 3; ++ i)
		{
			Node::Triplet const & t = n.triple[i];
			if (t.cousin != nullptr && t.mid_point != nullptr)
			{
				triplet_indices[true] = i;
				++ num_mid_points;
			}
			else 
			{
				triplet_indices[false] = i;
			}
		}
		
		// Step 2: Given the number of mid-points, construct the faces possible from mid-points and corners. 
		switch (num_mid_points)
		{
			case 0: 
			{
				// Only the corners are available. Has the advantage that we know what the normal is.
				f.AddFace(* n.triple[0].corner, * n.triple[1].corner, * n.triple[2].corner, n.normal);
			}	break;
				
			case 1: 
			{
				// A single mid-point means that the triangle can be divided in two.
				int mid_point_index = triplet_indices[true];
				Assert(mid_point_index >= 0);
				
				Point & mid_point = ref(n.triple[mid_point_index].mid_point);
				AddFace(mid_point, * n.triple[mid_point_index].corner, * n.triple[TriMod(mid_point_index+1)].corner, f);
				AddFace(mid_point, * n.triple[TriMod(mid_point_index+2)].corner, * n.triple[mid_point_index].corner, f);
			}	break;
				
			case 2: 
			{
				// Two mid-points means a triangle between them and their common corner
				// and a quad made from the rest of the node. 
				int non_mid_point_index = triplet_indices[false];
				Assert(non_mid_point_index >= 0);
				
				// Generate good triangle.
				AddFace(* n.triple[TriMod(non_mid_point_index+2)].mid_point, * n.triple[TriMod(non_mid_point_index+1)].mid_point, * n.triple[non_mid_point_index].corner, f);
				
				// Generate wonky quad.
				AddFace(* n.triple[TriMod(non_mid_point_index+1)].corner, * n.triple[TriMod(non_mid_point_index+2)].corner, * n.triple[TriMod(non_mid_point_index+2)].mid_point, f);
				AddFace(* n.triple[TriMod(non_mid_point_index+1)].mid_point, * n.triple[TriMod(non_mid_point_index+2)].mid_point, * n.triple[TriMod(non_mid_point_index+2)].corner, f);
			}	break;
				
			case 3: 
			{
				for (int i = 0; i < 4; ++ i)
				{
					// TODO: Refactor 2 switch into 1?!?
					Point * child_corners[3];
					n.GetChildCorners(i, child_corners);
					AddFace(* child_corners[0], * child_corners[1], * child_corners[2], f);
				}
			}	break;
		}
	}

}
