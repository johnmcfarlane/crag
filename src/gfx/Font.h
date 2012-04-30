//
//  Font.h
//  crag
//
//  Created by John on 04/17/2010.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Texture.h"
#include "VertexBufferObject.h"


namespace gfx
{
	class Font
	{
	public:
		// types
		struct Vertex
		{
			geom::Vector2f pos;
			geom::Vector2f tex;
		};
		
		typedef VertexBufferObject<Vertex> VertexBufferObject;
		
		// functions
		Font(char const * filename, float scale = 1.f);
		~Font();
		
		operator bool() const;

		void Print(char const * text, geom::Vector2f const & position) const;

	private:
		void GenerateVerts(char const * text, geom::Vector2f const & position) const;
		void RenderVerts() const;
		
		void PrintChar(char c, geom::Vector2f & position) const;

		geom::Vector2i character_size;
		geom::Vector2f inv_scale;
		float scale_factor;
		Texture texture;
		mutable VertexBufferObject vbo;
	};
}
