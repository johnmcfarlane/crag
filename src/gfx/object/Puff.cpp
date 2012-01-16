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

#include "gfx/Program.h"
#include "gfx/Renderer.h"
#include "gfx/Scene.h"
#include "gfx/SphereMesh.h"
#include "gfx/SphereQuad.h"

#include "glpp/glpp.h"

#include "geom/Sphere.h"

#include "core/Random.h"


using namespace gfx;


Puff::Puff(Scalar spawn_volume)
: LeafNode(Layer::foreground, ProgramIndex::disk)
, _spawn_volume(spawn_volume)
, _radius(0)
, _color(0, 0, 0, 0)
{
	SetIsOpaque(false);
}

bool Puff::Init(Scene & scene)
{
	Time time = scene.GetTime();
	_spawn_time = time - (Random::sequence.GetUnit<double>() / 60.);
	return true;
}

gfx::Transformation const & Puff::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	Time time = renderer.GetScene().GetTime();
	Time age = CalculateAge(time);
	
	gfx::Transformation scale(Vector3(age * 0.75, 0., 0.), Matrix33::Identity(), _radius);
	
	scratch = model_view * scale;
	
	return scratch;
}

LeafNode::PreRenderResult Puff::PreRender(Renderer const & renderer) override
{
	Time time = renderer.GetScene().GetTime();
	Time age = CalculateAge(time);
	
	_radius = SphereRadiusFromVolume<Scalar, 3>(_spawn_volume);
	_radius += 0.75 * age;
	
	_color.r = 1.f;
	_color.g = 1.f;
	_color.b = 1.f;
	_color.a = std::min(1. / Square(_radius * 10.f), 1.);
	
	if (_color.a < 0.01f)
	{
		return remove;
	}

	return ok;
}

void Puff::Render(Renderer const & renderer) const
{
	//Transformation const & transformation = GetModelViewTransformation();

	gl::Disable(GL_CULL_FACE);

#if 0
	gl::SetColor(_color.GetArray());
	SphereMesh const & sphere_mesh = renderer.GetSphereMesh();
	sphere_mesh.Draw(0);
#else
	DiskProgram const & disk_program = static_cast<DiskProgram const &>(ref(renderer.GetProgram(ProgramIndex::disk)));
	Transformation const & model_view = GetModelViewTransformation();
	disk_program.SetUniforms(model_view, _color);
	
	SphereQuad const & disk_quad = renderer.GetDiskQuad();
	disk_quad.Draw();
#endif
	
	gl::Enable(GL_CULL_FACE);
}

Time Puff::CalculateAge(Time time) const
{
	Time age = time - _spawn_time;
	return age;
}
