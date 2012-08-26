//
//  VertexBufferObject.h
//  crag
//
//  Created by John on 12/11/09.
//  Copyright 2009 - 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "BufferObject.h"

#include "Color.h"


// Helper functions used by VertexBufferObject to operate on different vertex formats.
// Specialize these three functions for the specific vertex format used.
// TODO: Consider only having EnableClientState fn in which unused buffers are disabled.
template <typename VERTEX> void EnableClientState();
template <typename VERTEX> void DisableClientState();
template <typename VERTEX> void Pointer();


namespace gfx
{
	// Wrapper for OpenGL VBO.
	template<typename VERTEX> 
	class VertexBufferObject : public BufferObject<VERTEX, GL_ARRAY_BUFFER>
	{
		typedef BufferObject<VERTEX, GL_ARRAY_BUFFER> super;
		
	public:
		bool IsBound() const
		{
			return super::IsBound();
		}
		
		// Enable the appropriate client states and set the buffer offsets.
		void Activate() const
		{
			assert(IsBound());
			
			EnableClientState<VERTEX>();
			Pointer<VERTEX>();
			
			GL_VERIFY;
		}

		// Disable the enabled client states.
		void Deactivate() const
		{
			assert(IsBound());
			DisableClientState<VERTEX>();
		}

		void Draw(GLenum mode, GLint first, GLsizei count) const
		{
			assert(IsBound());
			GL_CALL(glDrawArrays(mode, first, count));
		}
		
		void DrawTris(GLint first, GLsizei count) const
		{
			assert((count % 3) == 0);
			Draw(GL_TRIANGLES, first, count);
		}
		
		void DrawStrip(GLint first, GLsizei count) const
		{
			assert(count >= 3);
			Draw(GL_TRIANGLE_STRIP, first, count);
		}
		
		void DrawQuads(GLint first, GLsizei count) const
		{
			assert((count % 4) == 0);
			Draw(GL_QUADS, first, count);
		}
	};

	// Pointer helpers
	template <typename VERTEX, int SIZE, geom::Vector<float, SIZE> VERTEX::* MEMBER>
	void VertexPointer()
	{
		VERTEX * null_vert = nullptr;
		GL_CALL(glVertexPointer(SIZE, GL_FLOAT, sizeof(VERTEX), & (null_vert->* MEMBER)));
	}
	
	template <typename VERTEX, geom::Vector3f VERTEX::* MEMBER>
	void NormalPointer()
	{
		VERTEX * null_vert = nullptr;
		glNormalPointer(GL_FLOAT, sizeof(VERTEX), & (null_vert->* MEMBER));
	}
	
	template <typename VERTEX, Color4b VERTEX::* MEMBER>
	void ColorPointer()
	{
		VERTEX * null_vert = nullptr;
		GL_CALL(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VERTEX), & (null_vert->* MEMBER)));
	}
	
	template <typename VERTEX, int SIZE, geom::Vector<float, SIZE> VERTEX::* MEMBER>
	void TexCoordPointer()
	{
		VERTEX * null_vert = nullptr;
		GL_CALL(glTexCoordPointer(SIZE, GL_FLOAT, sizeof(VERTEX), & (null_vert->* MEMBER)));
	}
}
