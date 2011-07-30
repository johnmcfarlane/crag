/*
 *  BufferObject.h
 *  Crag
 *
 *  Created by John on 11/27/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Name.h"


namespace gl
{
	
	////////////////////////////////////////////////////////////////////////////////
	// BufferObject helper declarations
	
	template <typename ELEMENT, GLenum TARGET> class BufferObject;
	
	template <typename ELEMENT, GLenum TARGET> void DeleteBuffer(BufferObject<ELEMENT, TARGET> & bo);
	template <typename ELEMENT, GLenum TARGET> void GenBuffer(BufferObject<ELEMENT, TARGET> & bo);
	template <typename ELEMENT, GLenum TARGET> void BindBuffer(BufferObject<ELEMENT, TARGET> const & bo);
	
	
	////////////////////////////////////////////////////////////////////////////////
	// BufferObject class
	
	// Base class for IBO and VBO.
	template <typename ELEMENT, GLenum TARGET> class BufferObject : public Name<TARGET>
	{
		typedef Name<TARGET> super;
	public:
		bool IsBound() const
		{
			assert(super::IsInitialized());
			return super::id == GetBinding<TARGET>();
		}
		
		friend void GenBuffer<ELEMENT, TARGET>(BufferObject & bo);
		friend void DeleteBuffer<ELEMENT, TARGET>(BufferObject & bo);
		friend void BindBuffer<ELEMENT, TARGET>(BufferObject const & bo);
	};
				  

	////////////////////////////////////////////////////////////////////////////////
	// BufferObject helper definitions

	template <typename ELEMENT, GLenum TARGET> void GenBuffer(BufferObject<ELEMENT, TARGET> & bo)
	{
		assert(! bo.IsInitialized());
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glGenBuffersARB(1, & bo.id));
#else
		GLPP_CALL(glGenBuffers(1, & bo.id));
#endif
	}
	
	template <typename ELEMENT, GLenum TARGET> void DeleteBuffer(BufferObject<ELEMENT, TARGET> & bo)
	{
		assert(bo.IsInitialized());
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glDeleteBuffersARB(1, & bo.id));
#else
		GLPP_CALL(glDeleteBuffers(1, & bo.id));
#endif
		bo.id = 0;
	}
	
	template <typename ELEMENT, GLenum TARGET> void BindBuffer(BufferObject<ELEMENT, TARGET> const & bo)
	{
		assert(bo.IsInitialized());
		assert(! bo.IsBound());
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glBindBufferARB(TARGET, bo.id)); 
#else
		GLPP_CALL(glBindBuffer(TARGET, bo.id)); 
#endif
	}
	
	template <typename ELEMENT, GLenum TARGET> void UnbindBuffer(BufferObject<ELEMENT, TARGET> const & bo)
	{
		assert(bo.IsBound());
#if ! defined(NDEBUG)
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glBindBufferARB(TARGET, 0)); 
#else
		GLPP_CALL(glBindBuffer(TARGET, 0)); 
#endif
#endif
	}
	
	template <typename ELEMENT, GLenum TARGET> void BufferData(BufferObject<ELEMENT, TARGET> & bo, GLsizeiptr num, ELEMENT const * array = nullptr)
	{
		assert(bo.IsBound());
		GLsizeiptr size = sizeof(ELEMENT) * num;
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glBufferDataARB(TARGET, size, array, STREAM_DRAW));
#else
		GLPP_CALL(glBufferData(TARGET, size, array, STREAM_DRAW));
#endif
	}
	
	template <typename ELEMENT, GLenum TARGET> void BufferSubData(BufferObject<ELEMENT, TARGET> & bo, GLsizeiptr num, ELEMENT const * array)
	{
		assert(bo.IsBound());
		GLsizeiptr size = sizeof(ELEMENT) * num;
#if defined(GLPP_USE_ARB)
		GLPP_CALL(glBufferSubDataARB(TARGET, 0, size, array));
#else
		GLPP_CALL(glBufferSubData(TARGET, 0, size, array));
#endif
	}
}
