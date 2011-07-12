//
//  Vertex.h
//  crag
//
//  Created by John McFarlane on 7/4/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Vector2.h"
#include "geom/Vector3.h"


namespace gl
{
	
	////////////////////////////////////////////////////////////////////////////////
	// Vertex types.
	
	struct Vertex2dTex
	{
		Vector2f pos;
		Vector2f tex;
	};
	
	struct Vertex3d
	{
		Vector3f pos;
	};
	
	struct Vertex3dTex
	{
		Vector3f pos;
		Vector2f tex;
	};
	
	struct Vertex3dNorm
	{
		Vector3f pos;
		Vector3f norm;
	};
	
	// For spheres where the normal IS the position.
	struct Vertex3dNormHybrid
	{
		Vector3f pos;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// EnableClientState
	
	template <typename VERTEX> void EnableClientState();
	
	template <> inline void EnableClientState<Vertex2dTex>()
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
	}
	template <> inline void EnableClientState<Vertex3d>()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
	}
	template <> inline void EnableClientState<Vertex3dTex>()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	template <> inline void EnableClientState<Vertex3dNorm>()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
	}
	template <> inline void EnableClientState<Vertex3dNormHybrid>()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// DisableClientState
	
	template <typename VERTEX> void DisableClientState();
	
	template <> inline void DisableClientState<Vertex2dTex>()
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	template <> inline void DisableClientState<Vertex3d>()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	template <> inline void DisableClientState<Vertex3dTex>()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	template <> inline void DisableClientState<Vertex3dNorm>()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	template <> inline void DisableClientState<Vertex3dNormHybrid>()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Pointer
	
	template <typename VERTEX> void Pointer();
	
	template <> inline void Pointer<Vertex2dTex>()
	{
		const Vertex2dTex * null_vert = 0;
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex2dTex), & null_vert->tex);
	}
	template <> inline void Pointer<Vertex3d>()
	{
		const Vertex3d * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3d), & null_vert->pos);
	}
	template <> inline void Pointer<Vertex3dTex>()
	{
		const Vertex3dTex * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3dTex), & null_vert->tex);
	}
	template <> inline void Pointer<Vertex3dNorm>()
	{
		const Vertex3dNorm * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dNorm), & null_vert->pos);
		glNormalPointer(GL_FLOAT, sizeof(Vertex3dNorm), & null_vert->norm);
	}
	template <> inline void Pointer<Vertex3dNormHybrid>()
	{
		const Vertex3dNormHybrid * null_vert = 0;
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex3dNormHybrid), & null_vert->pos);
		glNormalPointer(GL_FLOAT, sizeof(Vertex3dNormHybrid), & null_vert->pos);
	}
	
}
