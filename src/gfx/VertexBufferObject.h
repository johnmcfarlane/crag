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
		VertexBufferObject() = default;
		
		VertexBufferObject(VertexBufferObject && rhs)
		: super(std::move(rhs))
		{
		}
		
		VertexBufferObject & operator=(VertexBufferObject && rhs)
		{
			super::operator=(std::move(rhs));
			return * this;
		}
		
		bool IsBound() const
		{
			return super::IsBound();
		}
		
		// Enable the appropriate client states and set the buffer offsets.
		void Activate() const
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsBound());
			
			EnableClientState<VERTEX>();
			Pointer<VERTEX>();
			
			GL_VERIFY;
		}

		// Disable the enabled client states.
		void Deactivate() const
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsBound());

			DisableClientState<VERTEX>();
		}

		void Draw(GLenum mode, GLint first, GLsizei count) const
		{
			CRAG_VERIFY(* this);
			CRAG_VERIFY_TRUE(IsBound());

			GL_CALL(glDrawArrays(mode, first, count));
		}
		
		void DrawTris(GLint first, GLsizei count) const
		{
			CRAG_VERIFY_EQUAL(count % 3, 0);

			Draw(GL_TRIANGLES, first, count);
		}
		
		void DrawStrip(GLint first, GLsizei count) const
		{
			CRAG_VERIFY_OP(count, >=, 3);

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
		static constexpr int dimension = 1;
	};

	template <>
	struct TypeInfo<int>
	{
		static constexpr GLenum type = GL_INT;
		static constexpr bool normalized = false;
		static constexpr int dimension = 1;
	};

	template <>
	struct TypeInfo<unsigned char>
	{
		static constexpr GLenum type = GL_UNSIGNED_BYTE;
		static constexpr bool normalized = false;
		static constexpr int dimension = 1;
	};
	
	template <typename Element>
	struct TypeInfo<Color4<Element>>
	{
		typedef TypeInfo<Element> ElementInfo;
		
		static constexpr GLenum type = ElementInfo::type;
		static constexpr bool normalized = ElementInfo::normalized;
		static constexpr int dimension = 4;
	};
	
	template <typename Element, int Dimension>
	struct TypeInfo<geom::Vector<Element, Dimension>>
	{
		typedef TypeInfo<Element> ElementInfo;
		
		static constexpr GLenum type = ElementInfo::type;
		static constexpr bool normalized = ElementInfo::normalized;
		static constexpr int dimension = Dimension;
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// GL attribute helper functions
	
	template <unsigned index, typename Vertex, typename MemberType, MemberType Vertex::* _member>
	void VertexAttribPointer()
	{
		//typedef TypeInfo<typename Vector::Scalar> TypeInfo;
		typedef TypeInfo<MemberType> TypeInfo;

		Vertex const * null_vert = nullptr;
		
		auto constexpr size = TypeInfo::dimension;
		auto constexpr type = TypeInfo::type;
		auto constexpr normalized = TypeInfo::normalized;
		auto constexpr stride = sizeof(Vertex);
		auto pointer = & (null_vert->* _member);
		
		GL_CALL(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
	}
}
