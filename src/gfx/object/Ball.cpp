//
//  gfx/object/Ball.cpp
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "gfx/axes.h"
#include "gfx/Program.h"
#include "gfx/Engine.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"

using namespace gfx;

DEFINE_POOL_ALLOCATOR(Ball);

Ball::Ball(Init const & init, Transformation const & local_transformation, float radius, Color4f const & color)
: Object(init, local_transformation, Layer::opaque, true)
, _color(color)
, _radius(radius)
{
	CRAG_VERIFY_EQUAL(_color.a, 1);
	
	auto & resource_manager = crag::core::ResourceManager::Get();

	auto const & sphere_program = * resource_manager.GetHandle<DiskProgram>("SphereProgram");
	SetProgram(& sphere_program);
	
	auto & sphere_quad = * resource_manager.GetHandle<Quad>("SphereQuadVbo");
	SetVboResource(& sphere_quad);
}

void Ball::UpdateModelViewTransformation(Transformation const & model_view)
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	Transformation model_view_transformation = sphere_quad.CalculateModelViewTransformation(model_view, _radius);

	SetModelViewTransformation(model_view_transformation);

	_shadow_model_view_transformation = Transformation(model_view.GetTranslation(), model_view.GetRotation());
}

Transformation const & Ball::GetShadowModelViewTransformation() const
{
	return _shadow_model_view_transformation;
}

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);
	
	range[0] = depth - _radius;
	range[1] = depth + _radius;
	
	return true;
}

// generates a cone frustum with thin edge hugging sphere and wide edge 
// projected away from light source
bool Ball::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	auto constexpr shadow_num_strips = 32;
	auto constexpr num_vertices_per_strip = 6;
	auto constexpr num_vertices = shadow_num_strips * num_vertices_per_strip;

	std::array<ShadowVolume::Vertex, num_vertices> vertices;
	{
		auto constexpr shadow_length_factor = 1000.f;
		auto constexpr disc_area = geom::SphereProperties<Scalar, 2>::Area(1.f);
		auto /*constexpr*/ disc_approximation_area = geom::GetPolygonArea<Scalar>(shadow_num_strips);
		auto /*constexpr*/ radius_correction = disc_area / disc_approximation_area;
	
		auto light_position = light.GetModelTransformation().GetTranslation();
		auto transformation = GetModelTransformation();
		auto position = transformation.GetTranslation();
		auto light_to_ball = geom::Inverse(transformation.GetRotation()) * (position - light_position);
		auto light_to_ball_distance = geom::Length(light_to_ball);
		auto light_to_ball_direction = light_to_ball / light_to_ball_distance;

		// from ball center, angle between light and shadow contact on surface
		auto ratio = _radius / light_to_ball_distance;
		if (ratio > 1.f)
		{
			// light is inside the sphere; shadow is everywhere
			return false;
		}
		
		auto angle = std::asin(ratio);

		auto near_offset_from_ball = - std::sin(angle) * _radius;
		auto shadow_length = _radius * shadow_length_factor;
		auto shadow_depth = std::cos(angle) * shadow_length;
		auto far_offset_from_ball = shadow_depth + near_offset_from_ball;
		
		auto near_center = light_to_ball_direction * near_offset_from_ball;
		auto near_disc_radius = std::cos(angle) * _radius;

		auto far_disc_radius = radius_correction * near_disc_radius * (light_to_ball_distance + far_offset_from_ball) / (light_to_ball_distance - _radius);		
		auto far_center = light_to_ball_direction * far_offset_from_ball;

		auto shadow_rotation = gfx::Rotation(light_to_ball_direction, gfx::Direction::forward);
		auto cos_axis = gfx::GetAxis(shadow_rotation, gfx::Direction::right);
		auto sin_axis = gfx::GetAxis(shadow_rotation, gfx::Direction::up);
	
		for (auto index = 0; index < shadow_num_strips; ++ index)
		{
			auto constexpr step_angle = Scalar(PI * 2. / shadow_num_strips);
			Scalar angle[2] = { index * step_angle, (index + 1) * step_angle };

			Vector3 quad[4];
			quad[0] = near_center + near_disc_radius * (cos_axis * std::cos(angle[0]) + sin_axis * std::sin(angle[0]));
			quad[1] = near_center + near_disc_radius * (cos_axis * std::cos(angle[1]) + sin_axis * std::sin(angle[1]));
			quad[2] = far_center + far_disc_radius * (cos_axis * std::cos(angle[0]) + sin_axis * std::sin(angle[0]));
			quad[3] = far_center + far_disc_radius * (cos_axis * std::cos(angle[1]) + sin_axis * std::sin(angle[1]));
		
			auto index_base = index * num_vertices_per_strip;
			vertices[index_base + 0] = {{ quad[0] }};
			vertices[index_base + 1] = {{ quad[1] }};
			vertices[index_base + 2] = {{ quad[2] }};
			vertices[index_base + 3] = {{ quad[3] }};
			vertices[index_base + 4] = {{ quad[2] }};
			vertices[index_base + 5] = {{ quad[1] }};
		}
	}
	
	shadow_volume.Set(vertices);
	return true;
}

void Ball::Render(Engine const &) const
{
	// Pass rendering details to the shader program.
	DiskProgram const & sphere_program = static_cast<DiskProgram const &>(* GetProgram());
	Transformation const & transformation = GetModelViewTransformation();
	sphere_program.SetUniforms(transformation, _radius, _color);

	// Draw the quad.
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	sphere_quad.Draw();
}
