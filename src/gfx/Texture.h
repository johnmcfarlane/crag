//
//  Texture.h
//  crag
//
//  Created by John on 12/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "glHelpers.h"


namespace gfx
{

	////////////////////////////////////////////////////////////////////////////////
	// OpenGl texture object wrapper
	
	class Texture
	{
	public:
		OBJECT_NO_COPY(Texture);

		Texture() = default;
		Texture(Texture && rhs);
		~Texture();
		
		Texture & operator=(Texture && rhs);
		
		bool IsInitialized() const;
		bool IsBound() const;
		
		void Init();
		void Deinit();

		void Bind() const;
		void Unbind() const;
		
		void SetImage(GLsizei width, GLsizei height, GLvoid const * pixels);
		void SetSubImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels);
		
	private:
		GLuint _name = 0;
	};
}
