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
	class Model;
	class VertexBuffer;

	class RootNode : public Node
	{
	public:
		RootNode();
		RootNode(Model & _owner);
		RootNode(RootNode const & rhs);
		
		DUMP_OPERATOR_FRIEND_DECLARATION(RootNode);

		void Init(int init_seed, VertexBuffer & vertices);
		void Deinit(VertexBuffer & vertices);
		
		Model & GetOwner() const;

		void SetCenter(Vector3d const & center, double scale);
	private:
		void SetVertexCenter(Vertex & vert, Vector3d const & relative_pos, Vector3d const & center, double scale);
		
		Model * owner;
	};
}
