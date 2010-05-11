/*
 *  Skybox.h
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "glpp/Vbo_Types.h"
#include "glpp/Texture.h"


namespace gfx
{
	class Image;

	class Skybox
	{
		// Methods
	public:
		Skybox();
		virtual ~Skybox();

		void SetSide(int axis, int pole, gfx::Image const & image);

		void Draw() const;

	private:
		void InitSides();
		void InitVerts();
	
		// Attribs
		gl::TextureRgba8 sides[3][2];
		gl::Vbo3dTex vbo;
	};

}
