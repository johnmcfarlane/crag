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
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(BufferObject, self)
			if (self.IsInitialized())
			{
				CRAG_VERIFY_TRUE(glIsBuffer(self._name));
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		BufferObject()
		: _name(0)
		{
		}
		
		BufferObject(BufferObject const &) = delete;
		
		BufferObject(BufferObject && rhs)
		: _name(rhs._name)
		{
			rhs._name = 0;
		}
		
		~BufferObject()
		{
			if (IsInitialized())
			{
				Deinit();
			}
		}

		BufferObject & operator=(BufferObject const &) = delete;
		
		BufferObject & operator=(BufferObject && rhs)
		{
			std::swap(_name, rhs._name);
			return * this;
		}
		
		bool IsInitialized() const
		{
			return _name != 0;
		}
		
		void Init()
		{
			CRAG_VERIFY_FALSE(IsInitialized());
			GL_CALL(glGenBuffers(1, & _name));
		}
		
		void Deinit()
		{
			CRAG_VERIFY_TRUE(IsInitialized());
			GL_CALL(glDeleteBuffers(1, & _name));
			_name = 0;
		}
		
		bool IsBound() const
		{
			CRAG_VERIFY_TRUE(IsInitialized());
			return _name == GetBinding<TARGET>();
		}
		
		void Bind() const
		{
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_EQUAL(GetBinding<TARGET>(), 0u);
			GL_CALL(glBindBuffer(TARGET, _name));
		}
		
		void Unbind() const
		{
			CRAG_VERIFY_TRUE(IsBound());
#if ! defined(NDEBUG)
			GL_CALL(glBindBuffer(TARGET, 0));
#endif
		}
		
		void BufferData(GLsizeiptr num, ELEMENT const * array, GLenum usage)
		{
			CRAG_VERIFY_OP(num, >=, 0);
			CRAG_VERIFY_TRUE(IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * num;
			GL_CALL(glBufferData(TARGET, size, array, usage));
		}
		
		void BufferData(GLsizeiptr num, GLenum usage)
		{
			BufferData(num, static_cast<ELEMENT *>(nullptr), usage);
		}
		
		void BufferSubData(GLsizeiptr num, ELEMENT const * array)
		{
			CRAG_VERIFY_OP(num, >=, 0);
			CRAG_VERIFY_TRUE(IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * num;
			GL_CALL(glBufferSubData(TARGET, 0, size, array));
		}
		
	protected:
		GLuint _name;	
	};
}
