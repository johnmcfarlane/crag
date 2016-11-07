//
//  gfx/NonIndexedVboResource.h
//  crag
//
//  Created by John McFarlane on 2014-03-29.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"
#include "VboResource.h"
#include "VertexBufferObject.h"

namespace gfx
{
	// a vertex buffer object without accompanying index buffer;
	// contrast to an IndecedVboResource
	template<typename VERTEX, GLenum USAGE> 
	class NonIndexedVboResource : public VboResource
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		using Vertex = VERTEX;
	private:
		using VertexBufferObject = gfx::VertexBufferObject<Vertex>;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
	public:
		// verification
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(NonIndexedVboResource, self)
			CRAG_VERIFY_OP(self._max_num_vertices, >=, self._num_vertices);
			CRAG_VERIFY_OP(self._num_vertices, >=, 0);
			CRAG_VERIFY_EQUAL(self._num_vertices % 3, 0);

			CRAG_VERIFY_OP(static_cast<uintmax_t>(self._max_num_vertices), <=, std::numeric_limits<gfx::ElementIndex>::max());
			
			if (self._max_num_vertices > 0)
			{
				CRAG_VERIFY_TRUE(self._vbo.IsInitialized());
			}
			if (! self._vbo.IsInitialized())
			{
				CRAG_VERIFY_FALSE(self._max_num_vertices);
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
		
		// c'tors
		NonIndexedVboResource()
		{
			CRAG_VERIFY(* this);
		}
		
		NonIndexedVboResource(NonIndexedVboResource && rhs)
		: _vbo(std::move(rhs._vbo))
		, _num_vertices(rhs._num_vertices)
		, _max_num_vertices(rhs._max_num_vertices)
		{
			rhs._num_vertices = 0;
			rhs._max_num_vertices = 0;

			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
		}
		
		NonIndexedVboResource(std::vector<Vertex> const & vertices)
		{
			Set(vertices);
		}
		
		~NonIndexedVboResource() noexcept
		{
			CRAG_VERIFY(* this);
			
			if (IsInitialized())
			{
				_vbo.Deinit();
			}
		}
		
		// copy
		NonIndexedVboResource & operator=(NonIndexedVboResource && rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
			
			_vbo = std::move(rhs._vbo);
			
			std::swap(_num_vertices, rhs._num_vertices);
			std::swap(_max_num_vertices, rhs._max_num_vertices);
			
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);

			return * this;
		}

		// set data
		template <typename VertexArray>
		void Set(VertexArray const & vertices)
		{
			auto first = vertices.data();
			Set(first, first + vertices.size());
		}
		
		void Set(Vertex const * vertices_begin, Vertex const * vertices_end)
		{
			CRAG_VERIFY(* this);

			// lazily initialize GL objects
			if (! IsInitialized())
			{
				_vbo.Init();
			}
			
			// bind
			_vbo.Bind();
			
			// calculate number of vertices
			_num_vertices = core::get_index(vertices_begin, *vertices_end);

			// expand vertex buffer as necessary
			if (_num_vertices > _max_num_vertices)
			{
				_vbo.BufferData(_num_vertices, USAGE);
				_max_num_vertices = _num_vertices;
			}
			
			// write data
			_vbo.BufferSubData(_num_vertices, vertices_begin);

			// unbind
			_vbo.Unbind();

			CRAG_VERIFY(* this);
		}

		// get state
		bool empty() const
		{
			return _num_vertices == 0;
		}
		
		bool IsInitialized() const
		{
			CRAG_VERIFY(* this);
			return _vbo.IsInitialized();
		}
		
		bool IsBound() const
		{
			return _vbo.IsBound();
		}

		// rendering
		void Activate() const
		{
			CRAG_VERIFY(* this);

			_vbo.Bind();

			_vbo.Activate();

			CRAG_VERIFY(* this);
		}

		void Deactivate() const
		{
			CRAG_VERIFY(* this);
			ASSERT(IsBound());

			_vbo.Deactivate();
			_vbo.Unbind();

			CRAG_VERIFY(* this);
		}

		void Draw() const
		{
			DrawTris(0, _num_vertices);
		}

		void DrawTris(int start, int count) const
		{
			ASSERT(IsBound());
			_vbo.DrawTris(start, count);
		}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		VertexBufferObject _vbo;

		int _num_vertices = 0;
		int _max_num_vertices = 0;
	};
}
