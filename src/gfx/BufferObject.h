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
		OBJECT_NO_COPY(BufferObject);

	public:
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(BufferObject, self)
			if (self.IsBound())
			{
				CRAG_VERIFY_TRUE(glIsBuffer(self._name));
			}
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		BufferObject()
		: _name(0)
		{
			CRAG_VERIFY(* this);
		}
		
		BufferObject(BufferObject && rhs)
		: _name(rhs._name)
		{
			rhs._name = 0;

			CRAG_VERIFY(rhs);
			CRAG_VERIFY(* this);
		}
		
		~BufferObject()
		{
			CRAG_VERIFY(* this);

			if (IsInitialized())
			{
				Deinit();
			}
		}

		BufferObject & operator=(BufferObject && rhs)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY(rhs);

			std::swap(_name, rhs._name);

			CRAG_VERIFY(rhs);
			CRAG_VERIFY(* this);

			return * this;
		}
		
		bool IsInitialized() const
		{
			CRAG_VERIFY(* this);

			return _name != 0;
		}
		
		void Init()
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_FALSE(IsInitialized());

			GL_CALL(glGenBuffers(1, & _name));

			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY(* this);
		}
		
		void Deinit()
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());

			GL_CALL(glDeleteBuffers(1, & _name));
			_name = 0;

			CRAG_VERIFY_FALSE(IsInitialized());
			CRAG_VERIFY(* this);
		}
		
		bool IsBound() const
		{
			return _name && _name == GetBinding<TARGET>();
		}
		
		void Bind() const
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_EQUAL(GetBinding<TARGET>(), 0u);

			GL_CALL(glBindBuffer(TARGET, _name));

			CRAG_VERIFY_TRUE(IsBound());
			CRAG_VERIFY(* this);
		}
		
		void Unbind() const
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsBound());

#if ! defined(NDEBUG)
			GL_CALL(glBindBuffer(TARGET, 0));
			CRAG_VERIFY_FALSE(IsBound());
#endif

			CRAG_VERIFY(* this);
		}
		
		void BufferData(GLsizeiptr num, ELEMENT const * array, GLenum usage)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_TRUE(IsBound());
			CRAG_VERIFY_OP(num, >= , 0);
			GLsizeiptr size = sizeof(ELEMENT) * num;

			GL_CALL(glBufferData(TARGET, size, array, usage));

			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_TRUE(IsBound());
		}
		
		void BufferData(GLsizeiptr num, GLenum usage)
		{
			BufferData(num, static_cast<ELEMENT *>(nullptr), usage);
		}
		
		void BufferSubData(GLsizeiptr num, ELEMENT const * array)
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_TRUE(IsBound());
			CRAG_VERIFY_OP(num, >= , 0);
			GLsizeiptr size = sizeof(ELEMENT) * num;

			GL_CALL(glBufferSubData(TARGET, 0, size, array));

			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsInitialized());
			CRAG_VERIFY_TRUE(IsBound());
		}
		
	protected:
		GLuint _name;	
	};
}
