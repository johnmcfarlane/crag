/*
 *  RootNode.h
 *  Crag
 *
 *  Created by John on 3/14/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
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
		RootNode(RootNode const & rhs);	// undefined
	public:
		RootNode(Model & _owner);
		
		DUMP_OPERATOR_FRIEND_DECLARATION(RootNode);

		void Init(int init_seed, VertexBuffer & vertices);
		void Deinit(VertexBuffer & vertices);
		
		Model & GetOwner() const;

		void SetCenter(Vector3d const & center, double scale);
	private:
		void SetVertexCenter(Vertex & vert, Vector3d const & relative_pos, Vector3d const & center, double scale);
		
		Model & owner;
	};
}
