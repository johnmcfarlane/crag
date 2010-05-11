/*
 *  Light.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
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
		Light(sim::Vector3 const & pos, Color4f const & col, float a = 0, float b = 0, float c = 1, bool init_shadows = false);
	
		bool IsActive() const { return true; }
		bool GenerateShadowMaps() const;
	
		void SetPosition(sim::Vector3 const & p);
		sim::Vector3 const & GetPosition() const;

		void Draw(int light_id) const;

	private:
		sim::Vector3 position;
		Color4f color;
		float attenuation_a;
		float attenuation_b;
		float attenuation_c;
		bool shadows;
	};
}
