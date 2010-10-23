/*
 *  VertexBuffer.h
 *  Crag
 *
 *  Created by john on 5/16/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/scene/Vertex.h"

#include "smp/vector.h"


namespace form 
{
	class VertexBuffer : private smp::vector<Vertex>
	{
		typedef smp::vector<Vertex> Super;
		
	public:
		VertexBuffer(int max_num_verts);

		size_t Size() const { return size(); }
		Vertex const * GetArray() const { return & front(); }
		Vertex & PushBack();
		
		void Clear();
		//void ClearNormals();

		void NormalizeNormals();
		
		int GetIndex(Vertex const & v) const;

		//DUMP_OPERATOR_DECLARATION(VertexBuffer);
	};
}
