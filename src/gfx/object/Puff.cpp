//
//  gfx/object/Puff.cpp
//  crag
//
//  Created by John McFarlane on 2011-12-10.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Puff.h"

#include "gfx/Scene.h"
#include "gfx/Sphere.h"
#include "gfx/Renderer.h"

#include "glpp/glpp.h"

#include "geom/Sphere.h"

#include "core/Random.h"


using namespace gfx;


Puff::Puff(Scalar spawn_volume)
: LeafNode(Layer::foreground)
, _spawn_volume(spawn_volume)
, _radius(0)
, _color(0, 0, 0, 0)
{
	SetIsOpaque(false);
}

void Puff::Init(Scene const & scene)
{
	_sphere = & scene.GetSphere();
	_spawn_time = scene.GetTime() - (Random::sequence.GetUnit<double>() / 60.);
}

gfx::Transformation const & Puff::Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	sys::Time age = CalculateAge();
	
	Transformation scale(Vector3(age * 0.75, 0., 0.), Matrix33::Identity(), _radius);
	
	scratch = model_view * scale;
	
	return scratch;
}

LeafNode::PreRenderResult Puff::PreRender() override
{
	sys::Time age = CalculateAge();
	
	_radius = SphereRadiusFromVolume<Scalar, 3>(_spawn_volume);
	_radius += 0.5 * age;
	
	_color.r = static_cast<uint8_t>(255.99 * exp(- _radius * 0.01f));
	_color.g = static_cast<uint8_t>(255.99 * exp(- _radius * 0.10f));
	_color.b = static_cast<uint8_t>(255.99 * exp(- _radius * 1.00f));
	_color.a = static_cast<uint8_t>(255.99 * exp(- _radius * 8.00f));
	
	if (_color.a == 0)
	{
		return remove;
	}

	return ok;
}

void Puff::Render() const
{
	//Transformation const & transformation = GetModelViewTransformation();

	gl::Disable(GL_CULL_FACE);
	gl::SetColor(_color.GetArray());
	
	_sphere->Draw(0);

	gl::Enable(GL_CULL_FACE);
}

sys::Time Puff::CalculateAge() const
{
	Renderer const & renderer = Daemon::Ref();
	sys::Time time = renderer.GetScene().GetTime();
	sys::Time age = time - _spawn_time;
	return age;
}
