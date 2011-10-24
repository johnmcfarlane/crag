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

#include "form/defs.h"

#include "smp/vector.h"


namespace form 
{
	class VertexBuffer
	{
	public:
		// functions
		VertexBuffer(int max_num_verts);

		size_t Size() const { return _verts.size(); }
		Vertex const * GetArray() const { return & _verts.front(); }
		Vertex & PushBack();
		
		void Clear();

		int GetIndex(Vertex const & v) const;
		
		// variables
		smp::vector<Vertex> _verts;
	};
}
