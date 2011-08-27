/*
 *  Light.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Object.h"

#include "Color.h"

#include "sim/defs.h"


namespace gfx
{
	class Light : public Object
	{
	public:
		Light(Vector3f const & pos, Color4f const & col, float a = 0, float b = 0, float c = 1, bool init_shadows = false);
		
		bool IsActive() const;
		bool GenerateShadowMaps() const;
		
		void SetPosition(Vector3f const & p);
		Vector3f const & GetPosition() const;
		
		virtual void Render(Layer::type layer, Scene const & scene) const;
		
		virtual bool IsInLayer(Layer::type) const;
		
	private:
		Vector3f position;
		Color4f color;
		float attenuation_a;
		float attenuation_b;
		float attenuation_c;
		bool shadows;
	};
}
