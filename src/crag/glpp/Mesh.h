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
	public:
		void Init()
		{
			vbo.Init();
			ibo.Init();
		}
		
		void Bind() const
		{
			gl::Bind(& vbo);
			gl::Bind(& ibo);
		}
		
		void Resize(int num_verts, int num_indices)
		{
			vbo.Resize(num_verts);
			ibo.Resize(num_indices);
		}
		
		void SetVbo(int num, Vertex const * array)
		{
			gl::Bind(& vbo);
			vbo.Set(num, array);
		}
		
		void SetIbo(int num, GLuint const * array)
		{
			gl::Bind(& ibo);
			ibo.Set(num, array);
		}
		
		void Draw(int min_index, int max_index, GLenum mode = GL_TRIANGLES) const
		{
			// IBO
			gl::Bind(& ibo);
			glDrawElements (mode, max_index - min_index, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(sizeof(int) * min_index));
		}
		
		void Clear()
		{
			Bind();
			vbo.Set(0, nullptr);
			ibo.Set(0, nullptr);
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
		Vbo<Vertex> vbo;	// verts
		Ibo ibo;	// indices
	};

}
