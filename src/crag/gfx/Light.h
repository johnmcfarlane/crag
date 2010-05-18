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

#include "Color.h"

#include "sim/Defs.h"


namespace gfx
{
	class Light
	{
	public:
		Light(Vector3f const & pos, Color4f const & col, float a = 0, float b = 0, float c = 1, bool init_shadows = false);
	
		bool IsActive() const { return true; }
		bool GenerateShadowMaps() const;
	
		void SetPosition(Vector3f const & p);
		Vector3f const & GetPosition() const;

		void Draw(int light_id) const;

	private:
		Vector3f position;
		Color4f color;
		float attenuation_a;
		float attenuation_b;
		float attenuation_c;
		bool shadows;
	};
}
