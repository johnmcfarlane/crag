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
#include "Vertex.h"


namespace gl
{
	
	////////////////////////////////////////////////////////////////////////////////
	// Vbo types
	
	class Vbo2dTex : public Vbo<Vertex2dTex>
	{
	};

	class Vbo3d : public Vbo<Vertex3d>
	{
	};

	class Vbo3dTex : public Vbo<Vertex3dTex>
	{
	};

	class Vbo3dNorm : public Vbo<Vertex3dNorm>
	{
	};
	
}
