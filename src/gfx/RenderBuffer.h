//
//  RenderBuffer.h
//  crag
//
//  Created by John on 12/13/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

//#include "FrameBuffer.h"


namespace gfx 
{
	// Wrapper for OpenGL render buffer object.
	class RenderBuffer
	{
		OBJECT_NO_COPY(RenderBuffer);
	public:
		RenderBuffer();
		~RenderBuffer();
		
		bool IsInitialized() const;
		bool IsBound() const;
		
		void Init();
		void Deinit();
		
		void Bind();
		void Unbind();

		void ResizeForDepth(GLsizei width, GLsizei height);
		
		//friend void AttachToFrameBuffer(RenderBuffer * render_buffer);
		
	private:
		GLuint _id;
	};
}

