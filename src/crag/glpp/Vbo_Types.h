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
#include "geom/Vector2.h"
#include "geom/Vector3.h"


namespace gl
{
	
	////////////////////////////////////////////////////////////////////////////////
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
	
	template<> inline void Vbo<Vertex2dTex>::Activate() const
	{
		assert(IsBound());
		
		// enable vertex arrays
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glEnableClientState( GL_VERTEX_ARRAY );			
		
		const Vertex2dTex * null_vert = 0;
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->tex);
		
		GLPP_VERIFY;
	}
	
	template<> inline void Vbo<Vertex2dTex>::Deactivate() const
	{
		assert(IsBound());
		
		// disable vertex arrays
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
		
		GLPP_VERIFY;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// 3-float Position.
	
	//	struct Vertex3d
	//	{
	//		Vector3f pos;
	//	};
	//	
	//	class Vbo3d : public Vbo<Vertex3d>
	//	{
	//	public:
	//	};
	//	
	//	template<> inline void Vbo<Vertex3d>::Activate() const
	//	{
	//		assert(IsBound());
	//		
	//		// enable vertex arrays
	//		glEnableClientState( GL_VERTEX_ARRAY );
	//		
	//		const Vertex3d * null_vert = 0;
	//		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3d), & null_vert->pos);
	//		
	//		GLPP_VERIFY;
	//	}
	//	
	//	template<> inline void Vbo<Vertex3d>::Deactivate() const
	//	{
	//		assert(IsBound());
	//		
	//		// disable vertex arrays
	//		glDisableClientState( GL_VERTEX_ARRAY );
	//		
	//		GLPP_VERIFY;
	//	}
	
	
	////////////////////////////////////////////////////////////////////////////////
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
	
	template<> inline void Vbo<Vertex3dTex>::Activate() const
	{
		assert(IsBound());
		
		// enable vertex arrays
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		
		const Vertex3dTex * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->tex);
		
		GLPP_VERIFY;
	}
	
	template<> inline void Vbo<Vertex3dTex>::Deactivate() const
	{
		assert(IsBound());
		
		// disable vertex arrays
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
		
		GLPP_VERIFY;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// 3-float Position + 3-float Normal.
	
	struct Vertex3dNorm
	{
		Vector3f pos;
		Vector3f norm;
	};
	
	class Vbo3dNorm : public Vbo<Vertex3dNorm>
	{
	public:
	};
	
	template<> inline void Vbo<Vertex3dNorm>::Activate() const
	{
		assert(IsBound());
		
		// enable vertex arrays
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		
		const Vertex3dNorm * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dNorm), & null_vert->pos);
		glNormalPointer(GL_FLOAT, sizeof(Vertex3dNorm), & null_vert->norm);
		
		GLPP_VERIFY;
	}
	
	template<> inline void Vbo<Vertex3dNorm>::Deactivate() const
	{
		assert(IsBound());
		
		// disable vertex arrays
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
		
		GLPP_VERIFY;
	}
	
}
