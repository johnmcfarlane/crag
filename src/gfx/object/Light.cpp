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

#include "gfx/Engine.h"
#include "gfx/Scene.h"
#include "gfx/Debug.h"

#include "geom/origin.h"

#include "core/ConfigEntry.h"


using namespace gfx;


Light::Light(LeafNode::Init const & init, Transformation const & local_transformation, Color4f const & color)
: LeafNode(init, local_transformation, Layer::light)
, _color(color)
{
	Scene & scene = init.engine.GetScene();
	Light::List & lights = scene.GetLightList();
	lights.push_back(* this);
	
	VerifyObject(* this);
}

Light::~Light()
{
	Scene & scene = GetEngine().GetScene();
	Light::List & lights = scene.GetLightList();
	lights.remove(* this);
}

#if defined(VERIFY)
void Light::Verify() const
{
	super::Verify();

	VerifyTrue(Light::List::is_contained(* this));
	VerifyTrue(_color.a == 1.f);
}
#endif

void Light::SetColor(Color4f const & color)
{
	_color = Color4f(color.r, color.g, color.b, 1.f);
}

Color4f const & Light::GetColor() const
{
	return _color;
}

#if ! defined(NDEBUG)
LeafNode::PreRenderResult Light::PreRender()
{
	geom::rel::Vector3 light_position = GetParent()->GetModelTransformation().GetTranslation();
	Debug::Vector3 basis_position = geom::Cast<double>(light_position);
	Debug::AddBasis(basis_position, 1000000.);
	
	return ok;
}
#endif
