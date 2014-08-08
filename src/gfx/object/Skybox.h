//
//  Skybox.h
//  crag
//
//  Created by John on 11/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"

#include "gfx/Texture.h"
#include "gfx/VertexBufferObject.h"

namespace gfx
{
	class Image;
	class Scene;
	DECLARE_CLASS_HANDLE(Skybox);
	
	class Skybox : public Object
	{
	public:
		// Methods
		Skybox(Engine & engine);
		
		void SetSide(int axis, int pole, std::shared_ptr<Image> const & image);
		
		virtual void UpdateModelViewTransformation(Transformation const & model_view) final;
		void Render(Engine const & renderer) const override;
		
	private:
		// Attribs
		TextureResourceHandle sides[3][2];
	};
	
}
