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

#include "gfx/Renderer.h"
#include "gfx/Scene.h"
#include "gfx/Debug.h"

#include "core/ConfigEntry.h"


using namespace gfx;


Light::Light()
: LeafNode(Layer::light)
{
}

#if defined(VERIFY)
void Light::Verify() const
{
	super::Verify();

	VerifyTrue(Light::List::is_contained(* this));
}
#endif

bool Light::Init(Renderer & renderer, Color4f const & color)
{
	SetColor(color);

	Scene & scene = renderer.GetScene();
	Light::List & lights = scene.GetLightList();
	lights.push_back(* this);
	
	return true;
}

void Light::Deinit(Scene & scene)
{
	Light::List & lights = scene.GetLightList();
	lights.remove(* this);
}

void Light::SetColor(Color4f const & color)
{
	_color = Color4f(color.r, color.g, color.b, 1.f);
}

Color4f const & Light::GetColor() const
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
