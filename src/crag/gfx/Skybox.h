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

#include "Object.h"

#include "glpp/Vbo_Types.h"
#include "glpp/Texture.h"


namespace gfx
{
	class Image;
	class Scene;

	class Skybox : public Object
	{
		// Methods
	public:
		virtual void Init();
		virtual void Deinit();

		void SetSide(int axis, int pole, gfx::Image const & image);

		virtual void Render(Layer::type layer, Scene const & scene) const;
		virtual bool IsInLayer(Layer::type layer) const;

	private:
		void InitVerts();
	
		// Attribs
		gl::TextureRgba8 sides[3][2];
		gl::Vbo3dTex vbo;
	};

}
