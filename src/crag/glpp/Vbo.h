/*
 *  Vbo.h
 *  Crag
 *
 *  Created by John on 12/11/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "BufferObject.h"


namespace gl
{
	
	template<typename Vertex> class Vbo : public BufferObject<Vertex, gl::ARRAY_BUFFER>
	{
		typedef BufferObject<Vertex, gl::ARRAY_BUFFER> BaseClass;
		
	public:
		bool IsBound() const
		{
			return BaseClass::IsBound();	// slightly silly
		}
		
		// Must be individually defined for each type of Vertex; includes the glEnableClientState and gl*Pointer calls.
		inline void Activate() const;

		// Must be individually defined for each type of Vertex; includes the glDisableClientState calls.
		inline void Deactivate() const;

		void Draw(GLenum mode, GLint first, GLsizei count) const
		{
			assert(IsBound());
			GLPP_CALL(glDrawArrays(mode, first, count));
		}
		
		void DrawTris(GLint first, GLsizei count) const
		{
			Draw(GL_TRIANGLES, first, count);
		}
		
		void DrawStrip(GLint first, GLsizei count) const
		{
			Draw(GL_TRIANGLE_STRIP, first, count);
		}
		
		void DrawQuads(GLint first, GLsizei count) const
		{
			Draw(GL_QUADS, first, count);
		}
	};


	template<> inline GLuint Init<gl::ARRAY_BUFFER>()
	{
		GLuint id;
		GLPP_CALL(gl::GenBuffers(1, &id));
		assert(id != 0);
		return id;
	}

	template<> inline void Deinit<gl::ARRAY_BUFFER>(GLuint id)
	{
		assert (id != 0);
		GLPP_CALL(gl::GenBuffers(1, &id));
	}

	template<> inline void Bind<gl::ARRAY_BUFFER>(GLuint id)
	{
		GLPP_CALL(gl::BindBuffer(gl::ARRAY_BUFFER, id));
	}

	template<> inline GLuint GetBindingEnum<gl::ARRAY_BUFFER>()
	{
		return gl::ARRAY_BUFFER_BINDING;
	}

}
