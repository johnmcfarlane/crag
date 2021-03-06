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

namespace gfx
{
	class Image;
	class Scene;
	DECLARE_CLASS_HANDLE(Skybox);
	
	class Skybox : public Object
	{
	public:
		// Methods
		Skybox(Engine & engine, ResourceKey textures_key);
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Skybox);

		virtual void UpdateModelViewTransformation(Transformation const & model_view) final;
		void Render(Engine const & renderer) const override;
		
	private:
		// Attribs
		TextureCubeMapHandle _texture;
	};
	
}
