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

#include "LeafNode.h"

#include "glpp/Vbo_Types.h"
#include "glpp/Texture.h"


namespace gfx
{
	class Image;
	class Scene;
	
	class Skybox : public LeafNode
	{
		// Types
		typedef gl::Vbo3dTex Vbo;
		typedef gl::TextureRgba8 Texture;
		
		// Methods
	public:
		Skybox();
		
		void Init(Scene const & scene) override;
		void Deinit() override;
		
		void SetSide(int axis, int pole, Image const & image);
		
		Transformation const & Transform(Transformation const & model_view, Transformation & scratch) const override;
		virtual void Render(Transformation const & transformation, Layer::type layer) const;
		
	private:
		void InitVerts();
		
		// Attribs
		Texture sides[3][2];
		Vbo vbo;
	};
	
}
