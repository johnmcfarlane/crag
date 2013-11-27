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
	
	CRAG_VERIFY(* this);
}

Light::~Light()
{
	Scene & scene = GetEngine().GetScene();
	Light::List & lights = scene.GetLightList();
	lights.remove(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Light, object)
	CRAG_VERIFY(static_cast<Light::super const &>(object));

	CRAG_VERIFY_TRUE(Light::List::is_contained(object));
	CRAG_VERIFY_TRUE(object._color.a == 1.f);
CRAG_VERIFY_INVARIANTS_DEFINE_END

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
	Debug::AddBasis(light_position, 1000000.);
	
	return ok;
}
#endif
