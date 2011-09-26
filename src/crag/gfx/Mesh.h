/*
 *  GlBuffer.h
 *  Crag
 *
 *  Created by john on 10/7/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "glpp/Vbo.h"
#include "glpp/Ibo.h"


namespace gfx
{
	template<typename Vertex, gl::BufferDataUsage Usage> class Mesh
	{
		typedef gl::Vbo<Vertex> Vbo;
		typedef gl::Ibo Ibo;
	public:
		void Init()
		{
			GenBuffer(vbo);
			GenBuffer(ibo);
		}
		
		void Deinit()
		{
			DeleteBuffer(ibo);
			DeleteBuffer(vbo);
		}
		
		void Bind() const
		{
			BindBuffer(vbo);
			BindBuffer(ibo);
		}
		
		void Unbind() const
		{
			UnbindBuffer(vbo);
			UnbindBuffer(ibo);
		}
		
		void Resize(int num_verts, int num_indices, gl::BufferDataUsage usage)
		{
			BufferData(vbo, num_verts, usage);
			BufferData(ibo, num_indices, usage);
		}
		
		void SetVbo(int num, Vertex const * array)
		{
			BufferSubData(vbo, num, array);
		}
		
		void SetIbo(int num, GLuint const * array)
		{
			BufferSubData(ibo, num, array);
		}
		
		void Activate() const
		{
			Assert(IsBound());
			vbo.Activate();
		}
		
		void Deactivate() const
		{
			Assert(IsBound());
			vbo.Deactivate();
		}
		
		void Draw(GLenum mode, GLsizei count, GLuint first = 0) const
		{
			assert(mode == GL_TRIANGLES);
			DrawElements(ibo, mode, count, first);
		}
		
		bool IsBound() const
		{
			return vbo.IsBound() && ibo.IsBound();
		}

	#if defined(DUMP)
		friend std::ostream & operator << (std::ostream &out, Mesh const & mesh)
		{
			return out << "mesh:{" << mesh.vbo << ';' << mesh.ibo << '}';
		}	
	#endif
		
	protected:
		Vbo vbo;	// verts
		Ibo ibo;	// indices
	};

}
