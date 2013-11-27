//
//  VertexBuffer.h
//  crag
//
//  Created by john on 5/16/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Vertex.h"

#include "smp/vector.h"


namespace form 
{
	class VertexBuffer
	{
		// types
		typedef smp::vector<Vertex> VertexVector;
	public:
		// functions
		VertexBuffer(int max_num_verts);

		size_t Size() const { return _verts.size(); }
		Vertex const * GetArray() const { return & _verts.front(); }
		Vertex & PushBack(Vertex const & element);
		
		void Clear();

		int GetIndex(Vertex const & v) const;
		
		Vertex const & operator[] (int index) const;
		Vertex & operator[] (int index);
		
		VertexVector::const_iterator begin() const
		{
			return std::begin(_verts);
		}
		VertexVector::iterator begin()
		{
			return std::begin(_verts);
		}
		
		VertexVector::const_iterator end() const
		{
			return std::end(_verts);
		}
		VertexVector::iterator end()
		{
			return std::end(_verts);
		}
		
	private:
		// variables
		VertexVector _verts;
	};
}
