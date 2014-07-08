//
//  gfx/object/Planet.cpp
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"

#include "gfx/axes.h"
#include "gfx/Quad.h"

#include "core/ResourceManager.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

DEFINE_POOL_ALLOCATOR(Planet);

Planet::Planet(Engine & engine, Transformation const & local_transformation, Scalar radius)
: Object(engine, local_transformation, Layer::opaque)
, _sea_level(radius)
{
	auto & resource_manager = crag::core::ResourceManager::Get();
	
	auto const & sphere_quad = * resource_manager.GetHandle<Quad>("SphereQuadVbo");

	SetVboResource(& sphere_quad);
}

void Planet::SetRadiusMinMax(Scalar radius_min, Scalar radius_max)
{
	_radius_min = radius_min;
	_radius_max = radius_max;
}

void Planet::UpdateModelViewTransformation(Transformation const & model_view)
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	SetModelViewTransformation(sphere_quad.CalculateModelViewTransformation(model_view, _radius_max));
}

bool Planet::GetRenderRange(RenderRange & range) const 
{
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);
	
	range.y = depth + _radius_max;
	range.x = depth - _radius_max;
	
	return true;
}

void Planet::Render(Engine const &) const
{
	if (_sea_level < _radius_min)
	{
		return;
	}
}
