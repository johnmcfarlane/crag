/*
 *  form/PolyTraversal.h
 *  Crag
 *
 *  Created by John on 10/15/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#include "form/Formation.h"
#include "form/Vertex.h"
#include "core/VectorOps.h"


namespace form
{

	// Can provide base class for otherwise-incomplete custom functor,
	// or simply provide example of how to write a functor for ForEachPoly
	class PolyTraversalFunctor
	{
	public:
		enum Action {
			RECURSE,	// Keep mining deeper for smaller polys.
			STOP,		// Stop at this node and generate polys.
			CULL		// Don't draw anything.
		};
		
		Action OnEnterNode(Node const & node)
		{
			return (node.children == nullptr) ? STOP : RECURSE;
		}
		
		void OnExitNode(Node const & /*node*/)
		{
		}
		
		void operator()(Vertex & /*a*/, Vertex & /*b*/, Vertex & /*c*/, Vector3f const & /*normal*/)
		{
			// Functor is useless if it doesn't do anything 
			// with the polys that are presented to it.
			Assert(false);
		}
	};


	template<typename Functor> void PolyTraversalOnPoly (Functor & f, Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal)
	{
		f(a, b, c, normal);
	}

	template<typename Functor> void PolyTraversalOnPoly (Functor & f, Vertex & a, Vertex & b, Vertex & c)
	{
		Vector3f normal = TriangleNormal(c.pos, b.pos, a.pos);
		if (FastSafeNormalize(normal)) {
			PolyTraversalOnPoly(f, a, b, c, normal);
		}
	}

	template<typename Functor> void ForEachPoly(Node & node, Functor & f) 
	{
#if ! defined(NDEBUG)
		int triplet_indices[2] = { -1, -1 };	// nullptr/non-nullptr
#else
		int triplet_indices[2];	// nullptr/non-nullptr
#endif
		
		int num_mid_points = 0;
		
		if (node.children != nullptr) {
			for (int i = 0; i < 3; ++ i) {
				Node::Triplet const & t = node.triple[i];
				if (t.cousin != nullptr) {
					triplet_indices[true] = i;
					++ num_mid_points;
				}
				else {
					triplet_indices[false] = i;
				}
			}
		}
		
		switch (num_mid_points) {
			case 0: {
				PolyTraversalOnPoly(f, node.GetCorner(0), node.GetCorner(1), node.GetCorner(2), node.normal);
			}	break;
			
			case 1: {
				int mid_point_index = triplet_indices[true];
				Assert(mid_point_index >= 0);
				
				Vertex * mid_point = node.GetMidPoint(mid_point_index);
				PolyTraversalOnPoly(f, * mid_point, node.GetCorner(mid_point_index), node.GetCorner(TriMod(mid_point_index+1)));
				PolyTraversalOnPoly(f, * mid_point, node.GetCorner(TriMod(mid_point_index+2)), node.GetCorner(mid_point_index));
			}	break;
			
			case 2: {
				int non_mid_point_index = triplet_indices[false];
				Assert(non_mid_point_index >= 0);
				
				// Recurse for possible child (or simply draw points where the child would be.
				Node & child = node.children[non_mid_point_index];
				switch (f.OnEnterNode(child))
				{
					case PolyTraversalFunctor::RECURSE: {
						//Assert(child_stub.node != nullptr);
						ForEachPoly(child, f);
						f.OnExitNode(child);
					}	break;
						
					case PolyTraversalFunctor::STOP: {
						Vertex * child_corners[3];
						node.GetChildCorners(non_mid_point_index, child_corners);
						PolyTraversalOnPoly(f, * child_corners[0], * child_corners[1], * child_corners[2]);
					}	break;
						
					case PolyTraversalFunctor::CULL: {
					}	break;
				}
				
				// Generate wonky quad.
				PolyTraversalOnPoly(f, node.GetCorner(TriMod(non_mid_point_index + 1)), node.GetCorner(TriMod(non_mid_point_index + 2)), * node.GetMidPoint(TriMod(non_mid_point_index + 2)));
				PolyTraversalOnPoly(f, * node.GetMidPoint(TriMod(non_mid_point_index + 1)), * node.GetMidPoint(TriMod(non_mid_point_index + 2)), node.GetCorner(TriMod(non_mid_point_index + 2)));
			}	break;
			
			case 3: {
				for (int i = 0; i < 4; ++ i)
				{
					// TODO: Refactor 2 switch into 1?!?
					Node & child = node.children[i];
					switch (f.OnEnterNode(child))
					{
						case PolyTraversalFunctor::RECURSE: {
							//Assert(child.node != nullptr);
							ForEachPoly(child, f);
							f.OnExitNode(child);
						}	break;
							
						case PolyTraversalFunctor::STOP: {
							Vertex * child_corners[3];
							node.GetChildCorners(i, child_corners);
							PolyTraversalOnPoly(f, * child_corners[0], * child_corners[1], * child_corners[2]);
						}	break;
							
						case PolyTraversalFunctor::CULL: {
						}	break;
					}
				}
			}	break;
		}
	}
}
