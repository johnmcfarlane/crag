//
//  BufferObject.h
//  crag
//
//  Created by John McFarlane on 1/19/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "glHelpers.h"


namespace gfx
{
	
	////////////////////////////////////////////////////////////////////////////////
	// BufferObject class
	// 
	// Base class for IBO and VBO.
	template <typename ELEMENT, GLenum TARGET> 
	class BufferObject
	{
	public:
		BufferObject()
		: _name(0)
		{
		}
		
		~BufferObject()
		{
			assert(! IsInitialized());
		}

		bool IsInitialized() const
		{
			return _name != 0;
		}
		
		void Init()
		{
			assert(! IsInitialized());
			GL_CALL(glGenBuffers(1, & _name));
		}
		
		void Deinit()
		{
			assert(IsInitialized());
			GL_CALL(glDeleteBuffers(1, & _name));
			_name = 0;
		}
		
		bool IsBound() const
		{
			assert(IsInitialized());
			return _name == GetBinding<TARGET>();
		}
		
		void Bind() const
		{
			assert(IsInitialized());
			assert(GetBinding<TARGET>() == 0);
			GL_CALL(glBindBuffer(TARGET, _name));
		}
		
		void Unbind() const
		{
			assert(IsBound());
#if ! defined(NDEBUG)
			GL_CALL(glBindBuffer(TARGET, 0));
#endif
		}
		
		void BufferData(GLsizeiptr num, ELEMENT const * array, GLenum usage)
		{
			assert(IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * num;
			GL_CALL(glBufferData(TARGET, size, array, usage));
		}
		
		void BufferData(GLsizeiptr num, GLenum usage)
		{
			BufferData(num, static_cast<ELEMENT *>(nullptr), usage);
		}
		
		void BufferSubData(GLsizeiptr num, ELEMENT const * array)
		{
			assert(IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * num;
			GL_CALL(glBufferSubData(TARGET, 0, size, array));
		}
		
	protected:
		GLuint _name;	
	};
}
