/*
 *  VertexBuffer.h
 *  Crag
 *
 *  Created by john on 5/16/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/Pool.h"
#include "form/Vertex.h"


namespace form 
{
	class VertexBuffer : public Pool<Vertex>
	{
	public:
		VertexBuffer(int max_num_verts) 
		: Pool<Vertex>(max_num_verts)
		{
		}
		
		void ClearNormals();

		void NormalizeNormals();

		//DUMP_OPERATOR_DECLARATION(VertexBuffer);
	};
}
