//
//  Vbo.h
//  crag
//
//  Created by John on 12/11/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "BufferObject.h"


namespace gl
{

	// forward-declarations
	template <typename VERTEX> void EnableClientState();
	template <typename VERTEX> void DisableClientState();
	template <typename VERTEX> void Pointer();
	
	// Vbo - Vertex Buffer Object
	template<typename Vertex> class Vbo : public BufferObject<Vertex, gl::ARRAY_BUFFER>
	{
		typedef BufferObject<Vertex, gl::ARRAY_BUFFER> BaseClass;
		
	public:
		bool IsBound() const
		{
			return BaseClass::IsBound();	// slightly silly
		}
		
		// Must be individually defined for each type of Vertex; includes the glEnableClientState and gl*Pointer calls.
		inline void Activate() const
		{
			assert(IsBound());
			
			EnableClientState<Vertex>();
			Pointer<Vertex>();
			
			GLPP_VERIFY;
		}

		// Must be individually defined for each type of Vertex; includes the glDisableClientState calls.
		inline void Deactivate() const
		{
			assert(IsBound());
			
			DisableClientState<Vertex>();
			
			GLPP_VERIFY;
		}

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

}
