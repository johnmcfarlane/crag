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
#include "gfx/Engine.h"
#include "gfx/Quad.h"

#include "core/ResourceManager.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet(Engine & engine, Transformation const & local_transformation)
: Object(engine, local_transformation, Layer::opaque)
{
	auto & resource_manager = engine.GetResourceManager();
	
	auto sphere_quad = resource_manager.GetHandle<Quad>("SphereQuadVbo");

	SetVboResource(sphere_quad);
}

void Planet::SetMaxRadius(Scalar max_radius)
{
	_max_radius = max_radius;
}

void Planet::UpdateModelViewTransformation(Transformation const & model_view)
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	SetModelViewTransformation(sphere_quad.CalculateModelViewTransformation(model_view, _max_radius));
}

bool Planet::GetRenderRange(RenderRange & range) const 
{
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);
	
	range.y = depth + _max_radius;
	range.x = depth - _max_radius;
	
	return true;
}

void Planet::Render(Engine const &) const
{
}
