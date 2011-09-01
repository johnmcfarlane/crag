/*
 *  gfx/object/Skybox.h
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
		// Types
		typedef gl::Vbo3dTex Vbo;
		typedef gl::TextureRgba8 Texture;
		
		// Methods
	public:
		virtual void Init();
		virtual void Deinit();
		
		void SetSide(int axis, int pole, Image const & image);
		
		virtual void Render(Layer::type layer, Scene const & scene) const;
		virtual bool IsInLayer(Layer::type layer) const;
		
	private:
		void InitVerts();
		
		// Attribs
		Texture sides[3][2];
		Vbo vbo;
	};
	
}
