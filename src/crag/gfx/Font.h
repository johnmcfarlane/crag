/*
 *  gfx/Font.h
 *  Crag
 *
 *  Created by John on 04/17/2010.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Vector2.h"

#include "glpp/Texture.h"
#include "glpp/Vbo_Types.h"


namespace gfx
{
	class Font
	{
	public:
		Font(char const * filename, float scale = 1.f);
		~Font();
		
		operator bool() const;

		void Print(char const * text, Vector2f const & position) const;

	private:
		void GenerateVerts(char const * text, Vector2f const & position) const;
		void RenderVerts() const;
		
		void PrintChar(char c, Vector2f & position) const;

		Vector2i character_size;
		Vector2f inv_scale;
		float scale_factor;
		gl::TextureRgba8 texture;
		mutable gl::Vbo2dTex vbo;
	};
}
