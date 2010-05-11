/*
 *  Vbo.h
 *  Crag
 *
 *  Created by John on 12/11/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "BufferObject.h"


namespace gl
{
	
	template<typename Vertex> class Vbo : public BufferObject<Vertex, GL_ARRAY_BUFFER_ARB>
	{
		typedef BufferObject<Vertex, GL_ARRAY_BUFFER_ARB> BaseClass;
		
	public:
		bool IsBound() const
		{
			return BaseClass::IsBound();	// slightly silly
		}
		
		// Must be individually defined for each type of Vertex; includes the glEnableClientState and gl*Pointer calls.
		inline void Begin() const;

		// Must be individually defined for each type of Vertex; includes the glDisableClientState calls.
		inline void End() const;

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


	template<> inline GLuint Init<GL_ARRAY_BUFFER_ARB>()
	{
		GLuint id;
		GLPP_CALL(glGenBuffers(1, &id));
		assert(id != 0);
		return id;
	}

	template<> inline void Deinit<GL_ARRAY_BUFFER_ARB>(GLuint id)
	{
		assert (id != 0);
		GLPP_CALL(glDeleteBuffers(1, &id));
	}

	template<> inline void Bind<GL_ARRAY_BUFFER_ARB>(GLuint id)
	{
		GLPP_CALL(glBindBuffer(GL_ARRAY_BUFFER_ARB, id));
	}

	template<> inline GLuint GetBindingEnum<GL_ARRAY_BUFFER_ARB>()
	{
		return GL_ARRAY_BUFFER_BINDING_ARB;
	}

}
