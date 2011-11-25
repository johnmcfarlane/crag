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
, shadows(init_shadows)
{
}

bool Light::IsActive() const 
{ 
	return true; 
}

bool Light::GenerateShadowMaps() const
{
	return shadows;
}

void Light::Update(UpdateParams const & params)
{
	_position = params.position;
}

void Light::Render(Layer::type layer, Scene const & scene) const
{
	Assert (layer == Layer::light);
	
	ObjectMap const & lights = scene.GetObjectMap();
	int light_index = 0;
	for (ObjectMap::const_iterator vector_position = lights.begin(); ; ++ vector_position)
	{
		Object const * object = vector_position->second;
		if (object == this)
		{
			break;
		}
		Assert(vector_position != lights.end());
		
		if (object->IsInLayer(Layer::light))
		{
			++ light_index;
		}
	}
	
	int light_id = GL_LIGHT0 + light_index;
	Assert(light_id <= GL_LIGHT7);
	
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
