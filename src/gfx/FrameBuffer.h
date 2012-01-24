//
//  FrameBuffer.h
//  crag
//
//  Created by John on 12/12/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace gfx
{
	// Wrapper for OpenGL frame buffer object
	class FrameBuffer
	{
	public:
		FrameBuffer();
		~FrameBuffer();
		
		bool IsInitialized() const;
		bool IsBound() const;
		
		void Init();
		void Deinit();
		
		void Bind(GLuint id);
		
	private:
		GLuint _id;
	};
}
