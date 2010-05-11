/*
 *  TexVbo.h
 *  Crag
 *
 *  Created by John on 12/11/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  Specialization of a Vbo with a simple Position/Texture UV pairing
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Vbo.h"
#include "core/Vector2.h"
#include "core/Vector3.h"


namespace gl
{
		
	// 2-float Position + 2-float Texture uv.

	struct Vertex2dTex
	{
		Vector2f pos;
		Vector2f tex;
	};

	class Vbo2dTex : public Vbo<Vertex2dTex>
	{
	public:
	};

	template<> inline void Vbo<Vertex2dTex>::Begin() const
	{
		assert(IsBound());
		
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );	// Enable Vertex Arrays
		glEnableClientState( GL_VERTEX_ARRAY );			// Enable Vertex Arrays
		
		const Vertex2dTex * null_vert = 0;
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->tex);
		
		GLPP_VERIFY;
	}

	template<> inline void Vbo<Vertex2dTex>::End() const
	{
		assert(IsBound());
		
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );	// Enable Vertex Arrays
		glDisableClientState( GL_VERTEX_ARRAY );			// Enable Vertex Arrays
		
		GLPP_VERIFY;
	}


	// 3-float Position + 2-float Texture uv.

	struct Vertex3dTex
	{
		Vector3f pos;
		Vector2f tex;
	};

	class Vbo3dTex : public Vbo<Vertex3dTex>
	{
	public:
	};

	template<> inline void Vbo<Vertex3dTex>::Begin() const
	{
		assert(IsBound());
		
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );	// Enable Vertex Arrays
		glEnableClientState( GL_VERTEX_ARRAY );			// Enable Vertex Arrays
		
		const Vertex3dTex * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->tex);
		
		GLPP_VERIFY;
	}

	template<> inline void Vbo<Vertex3dTex>::End() const
	{
		assert(IsBound());
		
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );	// Enable Vertex Arrays
		glDisableClientState( GL_VERTEX_ARRAY );			// Enable Vertex Arrays
		
		GLPP_VERIFY;
	}

}
