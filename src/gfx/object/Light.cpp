//
//  Light.cpp
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Light.h"

#include "gfx/Scene.h"
#include "gfx/Debug.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "geom/Vector3.h"


using namespace gfx;


Light::Light(Vector3f const & color)
: LeafNode(Layer::light, ProgramIndex::none)
, _color(color.x, color.y, color.z)
{
}

bool Light::Init(Scene & scene)
{
	Light::List & lights = scene.GetLightList();
	lights.push_back(* this);
	
	return true;
}

void Light::Deinit(Scene & scene)
{
	Light::List & lights = scene.GetLightList();
	lights.remove(* this);
}

void Light::SetColor(Vector3f const & color)
{
	_color = color;
}

Vector3f const & Light::GetColor() const
{
	return _color;
}

#if ! defined(NDEBUG)
LeafNode::PreRenderResult Light::PreRender(Renderer const & renderer)
{
	Debug::Vector3 basis_position = GetParent()->GetModelTransformation().GetTranslation();
	Debug::AddBasis(basis_position, 1000000.);
	
	return ok;
}
#endif
