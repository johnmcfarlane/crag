//
//  gfx/IndexedVboResource.h
//  crag
//
//  Created by John McFarlane on 2013-12-12.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "VboResource.h"
#include "VertexBufferObject.h"
#include "IndexBufferObject.h"

namespace gfx
{
	// a vertex buffer object with accompanying index buffer
	template<typename VERTEX, GLenum USAGE> 
	class IndexedVboResource : public VboResource
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		typedef VERTEX Vertex;
		typedef gfx::Mesh<VERTEX, ElementIndex> Mesh;
	private:
		typedef gfx::VertexBufferObject<Vertex> VertexBufferObject;
		typedef gfx::IndexBufferObject IndexBufferObject;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
	public:
		// verification
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(IndexedVboResource, self)
			CRAG_VERIFY_OP(self._max_num_indices, >=, 0);
			CRAG_VERIFY_OP(self._max_num_vertices, >=, 0);
			CRAG_VERIFY_OP(self._num_indices, >=, 0);

			CRAG_VERIFY_OP(self._max_num_indices, >=, self._num_indices);
			CRAG_VERIFY_OP(static_cast<uintmax_t>(self._max_num_vertices), <=, std::numeric_limits<gfx::ElementIndex>::max());
			CRAG_VERIFY_FALSE(self._num_indices % 3);
			
			if (self._max_num_vertices > 0)
			{
				CRAG_VERIFY_TRUE(self._vbo.IsInitialized());
			}
			if (! self._vbo.IsInitialized())
			{
				CRAG_VERIFY_FALSE(self._max_num_vertices);
			}

			if (self._max_num_indices > 0)
			{
				CRAG_VERIFY_TRUE(self._ibo.IsInitialized());
			}
			if (! self._ibo.IsInitialized())
			{
				CRAG_VERIFY_FALSE(self._max_num_indices);
			}

			CRAG_VERIFY_EQUAL(self._vbo.IsInitialized(), self._ibo.IsInitialized());
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
		
		// c'tors
		IndexedVboResource()
		{
			CRAG_VERIFY(* this);
		}
		
		IndexedVboResource(IndexedVboResource && rhs)
		: _vbo(std::move(rhs._vbo))
		, _ibo(std::move(rhs._ibo))
		, _max_num_vertices(rhs._max_num_vertices)
		, _max_num_indices(rhs._max_num_indices)
		, _num_indices(rhs._num_indices)
		{
			rhs._max_num_vertices = 0;
			rhs._max_num_indices = 0;
			rhs._num_indices = 0;

			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
		}
		
		IndexedVboResource(Mesh const & mesh)
		{
			Set(mesh);
		}
		
		~IndexedVboResource()
		{
			CRAG_VERIFY(* this);
			
			if (IsInitialized())
			{
				_vbo.Deinit();
				_ibo.Deinit();
			}
		}
		
		// copy
		IndexedVboResource & operator=(IndexedVboResource && rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
			
			_vbo = std::move(rhs._vbo);
			_ibo = std::move(rhs._ibo);
			
			std::swap(_max_num_vertices, rhs._max_num_vertices);
			std::swap(_max_num_indices, rhs._max_num_indices);
			std::swap(_num_indices, rhs._num_indices);
			
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);

			return * this;
		}

		// set data
		void Set(Mesh const & mesh)
		{
			auto & vertices = mesh.GetVertices();
			auto & indices = mesh.GetIndices();
			
			Set(vertices.data(), vertices.data() + vertices.size(), indices.data(), indices.data() + indices.size());
		}
		
		// get state
		int GetNumIndices() const
		{
			return _num_indices;
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

			Bind();

			_vbo.Activate();

			CRAG_VERIFY(* this);
		}

		void Deactivate() const
		{
			CRAG_VERIFY(* this);
			ASSERT(IsBound());

			_vbo.Deactivate();
			Unbind();

			CRAG_VERIFY(* this);
		}

		void Draw() const
		{
			ASSERT(IsBound());
			_ibo.DrawElements(GL_TRIANGLES, _num_indices, 0);
		}

	private:
		void Bind() const
		{
			_vbo.Bind();
			_ibo.Bind();
		}
		
		void Unbind() const
		{
			_vbo.Unbind();
			_ibo.Unbind();
		}

		void Set(Vertex const * vertices_begin, Vertex const * vertices_end, ElementIndex const * indices_begin, ElementIndex const * indices_end)
		{
			CRAG_VERIFY(* this);

			// lazily initialize GL objects
			if (! IsInitialized())
			{
				_vbo.Init();
				_ibo.Init();
			}
			
			// bind
			Bind();
			
			// calculate number of vertices and indices
			auto num_vertices = std::distance(vertices_begin, vertices_end);
			_num_indices = std::distance(indices_begin, indices_end);

			// expand vertex buffer as necessary
			if (num_vertices > _max_num_vertices)
			{
				_vbo.BufferData(num_vertices, USAGE);
				_max_num_vertices = num_vertices;
			}
			
			// expand index buffer as necessary
			if (_num_indices > _max_num_indices)
			{
				_ibo.BufferData(_num_indices, USAGE);
				_max_num_indices = _num_indices;
			}

			// write data
			_vbo.BufferSubData(num_vertices, vertices_begin);
			_ibo.BufferSubData(_num_indices, indices_begin);

			// unbind
			Unbind();

			CRAG_VERIFY(* this);
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables

		VertexBufferObject _vbo;
		IndexBufferObject _ibo;

		int _max_num_vertices = 0;
		int _max_num_indices = 0;

		int _num_indices = 0;
	};
}
