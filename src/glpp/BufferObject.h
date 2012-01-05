//
//  BufferObject.h
//  crag
//
//  Created by John on 11/27/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

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
		typedef ELEMENT VertexType;
		
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
		glGenBuffers(1, & bo.id);
	}
	
	template <typename ELEMENT, GLenum TARGET> void DeleteBuffer(BufferObject<ELEMENT, TARGET> & bo)
	{
		assert(bo.IsInitialized());
		glDeleteBuffers(1, & bo.id);
		bo.id = 0;
	}
	
	template <typename ELEMENT, GLenum TARGET> void BindBuffer(BufferObject<ELEMENT, TARGET> const & bo)
	{
		assert(bo.IsInitialized());
		assert(! bo.IsBound());
		glBindBuffer(TARGET, bo.id);
	}
	
	template <typename ELEMENT, GLenum TARGET> void UnbindBuffer(BufferObject<ELEMENT, TARGET> const & bo)
	{
		assert(bo.IsBound());
#if ! defined(NDEBUG)
		glBindBuffer(TARGET, 0); 
#endif
	}
	
	template <typename ELEMENT, GLenum TARGET> void BufferData(BufferObject<ELEMENT, TARGET> & bo, GLsizeiptr num, ELEMENT const * array, BufferDataUsage usage)
	{
		assert(bo.IsBound());
		GLsizeiptr size = sizeof(ELEMENT) * num;
		glBufferData(TARGET, size, array, usage);
	}
	
	template <typename ELEMENT, GLenum TARGET> void BufferData(BufferObject<ELEMENT, TARGET> & bo, GLsizeiptr num, BufferDataUsage usage)
	{
		BufferData(bo, num, static_cast<ELEMENT *>(nullptr), usage);
	}
	
	template <typename ELEMENT, GLenum TARGET> void BufferSubData(BufferObject<ELEMENT, TARGET> & bo, GLsizeiptr num, ELEMENT const * array)
	{
		assert(bo.IsBound());
		GLsizeiptr size = sizeof(ELEMENT) * num;
		glBufferSubData(TARGET, 0, size, array);
	}
}
