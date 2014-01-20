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
			CRAG_VERIFY(self._vbo);
			CRAG_VERIFY(self._ibo);

			CRAG_VERIFY_OP(static_cast<uintmax_t>(self._num_vertices), <=, std::numeric_limits<gfx::ElementIndex>::max());
			CRAG_VERIFY_FALSE(self._num_indices % 3);

			CRAG_VERIFY_EQUAL(self._vbo.IsInitialized(), self._ibo.IsInitialized());
			if (self._vbo.IsInitialized())
			{
				CRAG_VERIFY_EQUAL(self._vbo.IsBound(), self._ibo.IsBound());
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
		
		// c'tors
		IndexedVboResource() = default;
		
		IndexedVboResource(IndexedVboResource && rhs)
		: _vbo(std::move(rhs._vbo))
		, _num_vertices(rhs._num_vertices)
		, _ibo(std::move(rhs._ibo))
		, _num_indices(rhs._num_indices)
		{
			rhs._num_vertices = 0;
			rhs._num_indices = 0;

			CRAG_VERIFY(* this);
		}
		
		// reserves buffer space
		IndexedVboResource(Mesh const & mesh)
			: IndexedVboResource(
				mesh.GetVertices().data(), mesh.GetVertices().data() + mesh.GetVertices().size(), 
				mesh.GetIndices().data(), mesh.GetIndices().data() + mesh.GetIndices().size())
		{
		}
		
		// reserves buffer space
		IndexedVboResource(int max_num_vertices, int max_num_indices)
		: _num_vertices(max_num_vertices)
		, _num_indices(max_num_indices)
		{
			Init();
			Bind();
			Reserve(max_num_vertices, max_num_indices);
			Unbind();
			
			ASSERT(! _vbo.IsBound());
			CRAG_VERIFY(* this);
		}
		
		IndexedVboResource(Vertex const * vertices_begin, Vertex const * vertices_end, ElementIndex const * indices_begin, ElementIndex const * indices_end)
		: _num_vertices(std::distance(vertices_begin, vertices_end))
		, _num_indices(std::distance(indices_begin, indices_end))
		{
			Init();
			Bind();
			Reserve(_num_vertices, _num_indices);
			Set(vertices_begin, indices_begin);
			Unbind();
			
			ASSERT(! _ibo.IsBound());
			CRAG_VERIFY(* this);
		}

		~IndexedVboResource()
		{
			CRAG_VERIFY(* this);
			
			if (IsInitialized())
			{
				Deinit();
			
				CRAG_VERIFY(* this);
			}
		}
		
		// copy
		IndexedVboResource & operator=(IndexedVboResource && rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);
			
			_vbo = std::move(rhs._vbo);
			_ibo = std::move(rhs._ibo);
			
			std::swap(_num_vertices, rhs._num_vertices);
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
		
		void Set(Vertex const * vertices_begin, Vertex const * vertices_end, ElementIndex const * indices_begin, ElementIndex const * indices_end)
		{
			CRAG_VERIFY(* this);

			_num_vertices = std::distance(vertices_begin, vertices_end);
			_num_indices = std::distance(indices_begin, indices_end);
			
			Bind();
			Set(vertices_begin, indices_begin);
			Unbind();

			CRAG_VERIFY(* this);
		}
		
		// get state
		int GetNumVertices() const
		{
			return _num_vertices;
		}
		
		int GetNumIndices() const
		{
			return _num_indices;
		}
		
		bool IsInitialized() const
		{
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
			ASSERT(IsInitialized());

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
		
		void Reserve(int num_vertices, int num_indices)
		{
			_vbo.BufferData(num_vertices, USAGE);
			_ibo.BufferData(num_indices, USAGE);
		}
		
		void Set(Vertex const * vertices, ElementIndex const * indices)
		{
			_vbo.BufferSubData(_num_vertices, vertices);
			_ibo.BufferSubData(_num_indices, indices);
		}
		
		void Init()
		{
			_vbo.Init();
			_ibo.Init();
		}

		void Deinit()
		{
			ASSERT(! IsBound());
			
			_vbo.Deinit();
			_ibo.Deinit();
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables

		VertexBufferObject _vbo;	// solid / shadow
		int _num_vertices = 0;

		IndexBufferObject _ibo;
		int _num_indices = 0;
	};
}
