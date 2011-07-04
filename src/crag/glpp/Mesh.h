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

#include "Vbo.h"
#include "Ibo.h"


namespace gl
{
	
	template<typename Vertex> class Mesh
	{
		typedef Vbo<Vertex> Vbo;
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
		
		void Resize(int num_verts, int num_indices)
		{
			BufferData(vbo, num_verts);
			BufferData(ibo, num_indices);
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
		
		void Draw(int min_index, int max_index, GLenum mode = GL_TRIANGLES) const
		{
			// IBO
			assert(IsBound());
			glDrawElements (mode, max_index - min_index, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(sizeof(int) * min_index));
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
