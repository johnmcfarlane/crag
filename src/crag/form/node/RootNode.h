/*
 *  RootNode.h
 *  Crag
 *
 *  Created by John on 3/14/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Node.h"


namespace form
{
	class RootNode & GetRootNode(Node & node);

	class Polyhedron;
	class PointBuffer;

	class RootNode : public Node
	{
	public:
		RootNode();
		RootNode(Polyhedron & _owner);
		RootNode(RootNode const & rhs);
		
		DUMP_OPERATOR_FRIEND_DECLARATION(RootNode);

		void Init(int init_seed, Point * points[4]);
		void GetPoints(Point * points[4]);
		void Deinit(PointBuffer & points);
		
		Polyhedron & GetOwner() const;

		//void SetCenter(Vector3d const & _center, double scale);
	private:
		static void SetPointCenter(Vector3f & point, Vector3d const & relative_pos, Vector3d const & point_center, double scale);
		
		Polyhedron * owner;
	};

}
