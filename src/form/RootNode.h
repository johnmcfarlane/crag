//
//  RootNode.h
//  crag
//
//  Created by John on 3/14/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Node.h"


namespace form
{
	class RootNode & GetRootNode(Node & node);

	class Polyhedron;
	class PointBuffer;

	// TODO: Consider removing RootNode and refer instead of the child quaterna;
	// TODO: then change Node* variables into relative addresses
	class RootNode : public Node
	{
	public:
		RootNode();
		RootNode(Polyhedron & _owner);
		RootNode(RootNode const & rhs);
		
		void Init(int init_seed, Point * points[4]);
		void GetPoints(Point * points[4]);
		void Deinit(PointBuffer & points);
		
		Polyhedron & GetOwner() const;
	private:
		static void SetPointCenter(geom::Vector3f & point, geom::Vector3d const & relative_pos, geom::Vector3d const & point_center, double scale);
		
		Polyhedron * owner;
	};

}
