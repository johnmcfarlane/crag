//
//  VertexBufferObject.h
//  crag
//
//  Created by John on 12/11/09.
//  Copyright 2009 - 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "BufferObject.h"

#include "Color.h"


// Helper functions used by VertexBufferObject to operate on different vertex formats.
// Specialize these three functions for the specific vertex format used.
// TODO: Consider only having EnableClientState fn in which unused buffers are disabled.
template <typename VERTEX> void EnableClientState();
template <typename VERTEX> void DisableClientState();
template <typename VERTEX> void Pointer();


namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	// Wrapper for OpenGL VBO.
	template<typename VERTEX> 
	class VertexBufferObject : public BufferObject<VERTEX, GL_ARRAY_BUFFER>
	{
		typedef BufferObject<VERTEX, GL_ARRAY_BUFFER> super;
		
	public:
		bool IsBound() const
		{
			return super::IsBound();
		}
		
		// Enable the appropriate client states and set the buffer offsets.
		void Activate() const
		{
			assert(IsBound());
			
			EnableClientState<VERTEX>();
			Pointer<VERTEX>();
			
			GL_VERIFY;
		}

		// Disable the enabled client states.
		void Deactivate() const
		{
			assert(IsBound());
			DisableClientState<VERTEX>();
		}

		void Draw(GLenum mode, GLint first, GLsizei count) const
		{
			assert(IsBound());
			GL_CALL(glDrawArrays(mode, first, count));
		}
		
		void DrawTris(GLint first, GLsizei count) const
		{
			assert((count % 3) == 0);
			Draw(GL_TRIANGLES, first, count);
		}
		
		void DrawStrip(GLint first, GLsizei count) const
		{
			assert(count >= 3);
			Draw(GL_TRIANGLE_STRIP, first, count);
		}
	};

	////////////////////////////////////////////////////////////////////////////////
	// GL attribute helper types
	
	template <typename S>
	struct TypeInfo;

	template <>
	struct TypeInfo<float>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr bool normalized = false;
	};

	template <>
	struct TypeInfo<unsigned char>
	{
		static constexpr GLenum type = GL_UNSIGNED_BYTE;
		static constexpr bool normalized = false;
	};
	

	////////////////////////////////////////////////////////////////////////////////
	// GL attribute helper functions
	
	template <unsigned index, typename Vertex, typename Vector, Vector Vertex::* _member>
	void VertexAttribPointer()
	{
		typedef TypeInfo<typename Vector::Scalar> TypeInfo;

		constexpr Vertex * null_vert = nullptr;
		
		auto constexpr size = Vector::Size();
		auto constexpr type = TypeInfo::type;
		auto constexpr normalized = TypeInfo::normalized;
		auto constexpr stride = sizeof(Vertex);
		auto constexpr pointer = & (null_vert->* _member);
		
		GL_CALL(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
	}
}
