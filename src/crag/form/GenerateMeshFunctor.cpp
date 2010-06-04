/*
 *  GenerateMeshFunctor.cpp
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "GenerateMeshFunctor.h"

#include "Mesh.h"


int GetTwizzle(form::Node const & n)
{
	form::Node const * parent = n.parent;
	if (parent != nullptr)
	{
		int index = & n - parent->children;
		return 2 * index;
	}
	else 
	{
		return 0;
	}
}


int CalcUvIndex(int index)
{
	return (index + 900) % 3;
}


void form::GenerateMeshFunctor::GenerateMesh(Node & node)
{
	Assert(IsLeaf(node));
	
	int triplet_indices[2] = { -1, -1 };	// NULL/non-NULL
	
	int num_mid_points = 0;
	for (int i = 0; i < 3; ++ i)
	{
		Node::Triplet const & t = node.triple[i];
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
	

	int twizzler = GetTwizzle(node);
	if (twizzler == -1)
	{
		return;
	}
	
	switch (num_mid_points)
	{
		case 0: {
			mesh.AddFace(* node.triple[0].corner, * node.triple[1].corner, * node.triple[2].corner, CalcUvIndex(twizzler), node.normal);
		}	break;
			
		case 1: {
			int mid_point_index = triplet_indices[true];
			Assert(mid_point_index >= 0);
			
			Point & mid_point = ref(node.triple[mid_point_index].mid_point);
			mesh.AddFace(mid_point, * node.triple[mid_point_index].corner, * node.triple[TriMod(mid_point_index+1)].corner, CalcUvIndex(mid_point_index+twizzler+2));
			mesh.AddFace(mid_point, * node.triple[TriMod(mid_point_index+2)].corner, * node.triple[mid_point_index].corner, CalcUvIndex(mid_point_index+twizzler+1));
		}	break;
			
		case 2: {
			int non_mid_point_index = triplet_indices[false];
			Assert(non_mid_point_index >= 0);
			
			// Generate good triangle.
			mesh.AddFace(* node.triple[TriMod(non_mid_point_index+1)].mid_point, * node.triple[TriMod(non_mid_point_index+2)].mid_point, * node.triple[non_mid_point_index].corner, CalcUvIndex(twizzler+non_mid_point_index+1));

			// Generate wonky quad.
			mesh.AddFace(* node.triple[TriMod(non_mid_point_index+1)].corner, * node.triple[TriMod(non_mid_point_index+2)].corner, * node.triple[TriMod(non_mid_point_index+2)].mid_point, CalcUvIndex(twizzler+non_mid_point_index+1));
			mesh.AddFace(* node.triple[TriMod(non_mid_point_index+1)].mid_point, * node.triple[TriMod(non_mid_point_index+2)].mid_point, * node.triple[TriMod(non_mid_point_index+2)].corner, CalcUvIndex(twizzler+non_mid_point_index+2));
		}	break;
			
		case 3: {
			for (int i = 0; i < 4; ++ i)
			{
				// TODO: Refactor 2 switch into 1?!?
				Point * child_corners[3];
				node.GetChildCorners(i, child_corners);
				mesh.AddFace(* child_corners[0], * child_corners[1], * child_corners[2], CalcUvIndex(2 * i));
			}
		}	break;
	}

	/*	Vertex & a = node.GetCorner(0);
	Vertex & b = node.GetCorner(1);
	Vertex & c = node.GetCorner(2);
	mesh.AddFace(a, b, c, node.normal);*/
}
