//
//  MeshBufferObject.h
//  crag
//
//  Created by john on 10/7/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "VertexBufferObject.h"
#include "IndexBufferObject.h"


namespace gfx
{
	// convenience class combines VBO of given vertex type, IBO and a given USAGE value
	template<typename VERTEX, GLenum USAGE> 
	class MeshBufferObject
	{
	public:
		// types
		typedef VERTEX Vertex;
		typedef ::gfx::VertexBufferObject<Vertex> VertexBufferObject;
		
		// functions
		void Init()
		{
			vbo.Init();
			ibo.Init();
		}
		
		void Deinit()
		{
			ibo.Deinit();
			vbo.Deinit();
		}
		
		void Bind() const
		{
			vbo.Bind();
			ibo.Bind();
		}
		
		void Unbind() const
		{
			vbo.Unbind();
			ibo.Unbind();
		}
		
		void Resize(int num_verts, int num_indices)
		{
			vbo.BufferData(num_verts, USAGE);
			ibo.BufferData(num_indices, USAGE);
		}
		
		void SetVbo(int num, Vertex const * array)
		{
			vbo.BufferSubData(num, array);
		}
		
		void SetIbo(int num, ElementIndex const * array)
		{
			ibo.BufferSubData(num, array);
		}
		
		void Activate() const
		{
			ASSERT(IsBound());
			vbo.Activate();
		}
		
		void Deactivate() const
		{
			ASSERT(IsBound());
			vbo.Deactivate();
		}
		
		void Draw(GLenum mode, GLsizei count, GLuint first = 0) const
		{
			assert(mode == GL_TRIANGLES);
			ibo.DrawElements(mode, count, first);
		}

		bool IsInitialized() const
		{
			CRAG_VERIFY(* this);
			return vbo.IsInitialized();
		}
		
		bool IsBound() const
		{
			CRAG_VERIFY(* this);
			return vbo.IsBound();
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(MeshBufferObject, self)
			CRAG_VERIFY(self.vbo);
			CRAG_VERIFY(self.ibo);

			ASSERT(self.vbo.IsInitialized() == self.ibo.IsInitialized());
			if (! self.vbo.IsInitialized())
			{
				return;
			}
			
			ASSERT(self.vbo.IsBound() == self.ibo.IsBound());
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif
		
	protected:
		VertexBufferObject vbo;	// verts
		IndexBufferObject ibo;	// indices
	};

}
