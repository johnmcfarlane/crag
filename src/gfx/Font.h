//
//  Font.h
//  crag
//
//  Created by John on 04/17/2010.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Texture2d.h"
#include "VertexBufferObject.h"


namespace gfx
{
	class Font
	{
		OBJECT_NO_COPY(Font);
	public:
		// types
		struct Vertex
		{
			geom::Vector2f pos;
			geom::Vector2f tex;
		};
		
		typedef ::gfx::VertexBufferObject<Vertex> VertexBufferObject;
		
		// functions
		Font(char const * filename, float scale = 1.f);
		Font(Font && rhs);
		
		operator bool() const;

		void Print(char const * text, geom::Vector2f const & position) const;

	private:
		void GenerateVerts(char const * text, geom::Vector2f const & position) const;
		void RenderVerts() const;
		
		void PrintChar(char c, geom::Vector2f & position) const;
		void PrintNewLine(geom::Vector2f & position) const;
		
		geom::Vector2i character_size;
		geom::Vector2f inv_scale;
		float scale_factor;
		Texture2d texture;
		mutable VertexBufferObject vbo;
	};
}
