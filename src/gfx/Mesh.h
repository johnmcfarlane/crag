//
//  Mesh.h
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
	template<typename VERTEX, GLenum USAGE> class Mesh
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
		
		void SetIbo(int num, GLuint const * array)
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
		
		bool IsBound() const
		{
			VerifyObject(* this);
			return vbo.IsBound() && ibo.IsBound();
		}

#if defined(VERIFY)
		void Verify() const
		{
			ASSERT(vbo.IsInitialized() == ibo.IsInitialized());
			if (! vbo.IsInitialized())
			{
				return;
			}
			
			ASSERT(vbo.IsBound() == ibo.IsBound());
		}
#endif
		
	protected:
		VertexBufferObject vbo;	// verts
		IndexBufferObject ibo;	// indices
	};

}
