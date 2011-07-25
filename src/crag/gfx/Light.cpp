/*
 *  Light.cpp
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Light.h"

#include "Color.h"
#include "Scene.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "geom/Vector3.h"


using gfx::Light;
using namespace gfx;


Light::Light(Vector3f const & pos, Color4f const & col, float a, float b, float c, bool init_shadows)
: position(pos)
, color(col)
, attenuation_a(a)
, attenuation_b(b)
, attenuation_c(c)
, shadows(init_shadows)
{
}

RenderStage::type Light::GetRenderStage() const
{
	return RenderStage::light;
}

bool Light::IsActive() const 
{ 
	return true; 
}

bool Light::GenerateShadowMaps() const
{
	return shadows;
}

void Light::SetPosition(Vector3f const & p)
{
	position = p;
}

Vector3f const & Light::GetPosition() const
{ 
	return position; 
}

void Light::Draw(Scene const & scene) const
{
	ObjectVector const & lights = scene.GetObjects(RenderStage::light);
	ObjectVector::const_iterator vector_position = std::find(lights.begin(), lights.end(), this);
	Assert(vector_position != lights.end());
	int light_index = vector_position - lights.begin();
	
	int light_id = GL_LIGHT0 + light_index;
	Assert(light_id <= GL_LIGHT7);
	
	GLPP_CALL(glLightfv(light_id, GL_AMBIENT, Color4f(0, 0, 0)));
	GLPP_CALL(glLightfv(light_id, GL_DIFFUSE, color));
	GLPP_CALL(glLightfv(light_id, GL_SPECULAR, Color4f(0, 0, 0)));
	
	GLPP_CALL(glLightfv(light_id, GL_CONSTANT_ATTENUATION, & attenuation_c));
	GLPP_CALL(glLightfv(light_id, GL_LINEAR_ATTENUATION, & attenuation_b));
	GLPP_CALL(glLightfv(light_id, GL_QUADRATIC_ATTENUATION, & attenuation_a));
	
	float l[4] = {
		position.x,
		position.y,
		position.z,
		1	// or is it this that makes it positional?
	};
	GLPP_CALL(glLightfv(light_id, GL_POSITION, l));
}
