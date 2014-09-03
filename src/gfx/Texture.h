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

#include "core/counted_object.h"

namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	// OpenGl texture object wrapper
	
	template <GLenum TARGET>
	class Texture : public crag::counted_object<Texture<TARGET>>
	{
	protected:
		OBJECT_NO_COPY(Texture);

		Texture(GLuint name = 0)
		: _name(name)
		{
		}
		
		Texture(Texture && rhs)
		: _name(rhs._name)
		{
			rhs._name = 0;
		}
		
		~Texture()
		{
			if (IsInitialized())
			{
				Deinit();
			}
		}
		
		Texture & operator=(Texture && rhs)
		{
			std::swap(_name, rhs._name);
			return * this;
		}
		
	public:
		bool IsInitialized() const
		{
			return _name != 0;
		}

		bool IsBound() const
		{
			ASSERT(IsInitialized());
			return GetBinding<TARGET>() == _name;
		}

		void Init() 
		{ 
			assert(! IsInitialized());
			GL_CALL(glGenTextures(1, & _name)); 
		}

		void Deinit()
		{
			assert(IsInitialized());
			GL_CALL(glDeleteTextures(1, & _name)); 
			_name = 0;
		}

		void Bind() const
		{
			assert(! IsBound());
			GL_CALL(glBindTexture(TARGET, _name)); 
		}

		void Unbind() const
		{ 
			assert(IsBound());
#if ! defined(NDEBUG)
			GL_CALL(glBindTexture(TARGET, 0)); 
#endif
		}
		
	private:
		GLuint _name = 0;
	};
}
