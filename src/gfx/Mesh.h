//
//  gfx/Mesh.h
//  crag
//
//  Created by John on 2014-01-07.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace gfx
{
	// CPU-side representation of a mesh using vertices and indices
	template <typename Vertex, typename Index = ElementIndex>
	class Mesh
	{
	public:
		// types
		typedef Vertex VertexType;
		typedef Index IndexType;
		typedef std::vector<VertexType> VertexArrayType;
		typedef std::vector<IndexType> IndexArrayType;
		
		// functions
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Mesh, self)
			CRAG_VERIFY_FALSE(self._indices.size() % 3);
			auto num_vertices = self._vertices.size();
			for (auto index : self._indices)
			{
				CRAG_VERIFY_OP(index, <, num_vertices);
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		Mesh() = default;
		
		Mesh(int num_vertices, int num_indices)
		: _vertices(num_vertices)
		, _indices(num_indices)
		{
			CRAG_VERIFY(* this);
		}
		
		Mesh(Mesh const &) = default;
		
		Mesh(Mesh && rhs)
		: _vertices(std::move(rhs._vertices))
		, _indices(std::move(rhs._indices))
		{
			CRAG_VERIFY(* this);
		}
		
		Mesh & operator=(Mesh const &) = default;
		
		Mesh & operator=(Mesh && rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);

			_vertices = std::move(rhs._vertices);
			_indices = std::move(rhs._indices);

			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
			return * this;
		}
		
		void Clear()
		{
			CRAG_VERIFY(* this);

			_vertices.clear();
			_indices.clear();

			CRAG_VERIFY(* this);
		}
		
		void Resize(int num_vertices, int num_indices)
		{
			CRAG_VERIFY(* this);
			
			_vertices.resize(num_vertices);
			_indices.resize(num_indices);
			
			CRAG_VERIFY(* this);
		}
		
		VertexArrayType & GetVertices()
		{
			return _vertices;
		}
		
		VertexArrayType const & GetVertices() const
		{
			return _vertices;
		}
		
		IndexArrayType & GetIndices()
		{
			return _indices;
		}

		IndexArrayType const & GetIndices() const
		{
			return _indices;
		}
		
	private:
		// variables
		VertexArrayType _vertices;
		IndexArrayType _indices;
	};
	
}
