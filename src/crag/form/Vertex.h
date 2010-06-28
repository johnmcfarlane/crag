/*
 *  Vertex.h
 *  Crag
 *
 *  Created by john on 5/1/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */
 
#pragma once

#include "defs.h"

//#define FORM_VERTEX_COLOR
#if defined(FORM_VERTEX_COLOR)
#include "gfx/Color.h"
#endif

//#define FORM_VERTEX_TEXTURE


namespace form
{

	class Vertex
	{
	public:
		// Attributes
		Vector3 pos;

#if defined(FORM_VERTEX_COLOR)
		gfx::Color4b color;
#endif // defined(FORM_VERTEX_COLOR)
		
		Vector3f norm;

#if defined(FORM_VERTEX_TEXTURE)
		Vector2f texture;
#endif // defined(FORM_VERTEX_TEXTURE)
	};
}
