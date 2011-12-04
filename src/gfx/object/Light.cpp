/*
 *  gfx/object/Light.cpp
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Light.h"

#include "gfx/Scene.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "geom/Vector3.h"


using gfx::Light;
using namespace gfx;


Light::Light(Vector3f const & pos, Color4f const & col, float a, float b, float c, bool init_shadows)
: LeafNode(Layer::light)
, _position(pos)
, color(col)
, attenuation_a(a)
, attenuation_b(b)
, attenuation_c(c)
, light_id(GL_NONE)
{
}

void Light::Init()
{
	for (int light_index = 0; light_index < 8; ++ light_index)
	{
		SlotMap bit = 1 << light_index;
		bool currently_used = (_used_slots & bit) != 0;
		Assert(currently_used == gl::IsEnabled(GL_LIGHT0 + light_index));
		
		if (! currently_used)
		{
			_used_slots |= bit;
			light_id = GL_LIGHT0 + light_index;

			Assert(! gl::IsEnabled(light_id));
			GLPP_CALL(gl::Enable(light_id));
			return;
		}
	}

	// Currently, we're only using two lights. The limit is eight.
	Assert(false);
}

void Light::Deinit()
{
	// Get the corresponding bit in the used slot map.
	int light_index = light_id - GL_LIGHT0;
	SlotMap bit = 1 << light_index;

	// Unset it.
	Assert((_used_slots & bit) != 0);
	_used_slots &= ~ bit;

	// Update the GL state.
	Assert(gl::IsEnabled(light_id));
	GLPP_CALL(gl::Disable(light_id));
}

bool Light::IsActive() const 
{ 
	return true; 
}

void Light::Update(UpdateParams const & params)
{
	_position = params.position;
}

void Light::Render(Layer::type layer, Scene const & scene) const
{
	Assert (layer == Layer::light);
	Assert(gl::IsEnabled(light_id));
	
	GLPP_CALL(glLightfv(light_id, GL_AMBIENT, Color4f(0, 0, 0)));
	GLPP_CALL(glLightfv(light_id, GL_DIFFUSE, color));
	GLPP_CALL(glLightfv(light_id, GL_SPECULAR, Color4f(0, 0, 0)));
	
	GLPP_CALL(glLightfv(light_id, GL_CONSTANT_ATTENUATION, & attenuation_c));
	GLPP_CALL(glLightfv(light_id, GL_LINEAR_ATTENUATION, & attenuation_b));
	GLPP_CALL(glLightfv(light_id, GL_QUADRATIC_ATTENUATION, & attenuation_a));
	
	Vector pos = _position - scene.GetPov().GetPosition();
	float l[4] = {
		float(pos.x),
		float(pos.y),
		float(pos.z),
		1	// or is it this that makes it positional?
	};
	GLPP_CALL(glLightfv(light_id, GL_POSITION, l));
}


Light::SlotMap Light::_used_slots = 0;
