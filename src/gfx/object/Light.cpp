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

#if defined(CRAG_GFX_LIGHT_DEBUG)
#if ! defined(CRAG_GFX_DEBUG)
#error Pointless definition of CRAG_GFX_LIGHT_DEBUG
#endif

#include "gfx/Debug.h"
#endif

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Light definitions

Light::Light(
	Engine & engine, Transformation const & local_transformation, 
	Color4f const & color, LightAttributes attributes,
	ObjectHandle exception)
: Object(engine, local_transformation, Layer::light)
, _color(color)
, _exception(nullptr)
, _attributes(attributes)
{
	if (exception.IsInitialized())
	{
		auto exception_object = engine.GetObject(exception);
		ASSERT(exception_object);
		
		if (exception_object)
		{
			_exception = & core::StaticCast<Object>(* exception_object);
		}
	}
	
	Scene & scene = engine.GetScene();
	CRAG_VERIFY(scene);
	scene.AddLight(* this);
	
	CRAG_VERIFY(* this);
}

Light::~Light()
{
	CRAG_VERIFY(* this);

	Scene & scene = GetEngine().GetScene();
	scene.RemoveLight(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Light, object)
	CRAG_VERIFY_TRUE(Light::List::is_contained(object));
	CRAG_VERIFY_TRUE(object._color.a == 1.f);
	CRAG_VERIFY_TRUE(object._attributes.type == LightType::point || object._attributes.type == LightType::search);
	CRAG_VERIFY_TRUE(object._attributes.resolution == LightResolution::vertex || object._attributes.resolution == LightResolution::fragment);
	CRAG_VERIFY(object._attributes.makes_shadow);
CRAG_VERIFY_INVARIANTS_DEFINE_END

bool Light::GetIsExtinguished() const
{
	return _is_extinguished;
}

void Light::SetIsExtinguished(bool is_extinguished)
{
	_is_extinguished = is_extinguished;
}

bool Light::GetIsLuminant() const
{
	return ! _is_extinguished && _color.r + _color.g + _color.b > 0.f;
}

void Light::SetColor(Color4f const & color)
{
	_color = Color4f(color.r, color.g, color.b, 1.f);
}

Color4f const & Light::GetColor() const
{
	return _color;
}

void Light::SetAngle(Vector2)
{
	DEBUG_BREAK("Pointless call. Intended?");
}

Vector2 Light::GetAngle() const
{
	return Vector2::Zero();
}

LightAttributes Light::GetAttributes() const
{
	return _attributes;
}

Object const * Light::GetException() const
{
	return _exception;
}

#if defined(CRAG_GFX_LIGHT_DEBUG)
PreRenderResult Light::PreRender()
{
	Vector3 intensity(_color.r, _color.g, _color.b);
	Debug::AddBasis(GetModelTransformation(), geom::Length(intensity));
	
	return ok;
}
#endif
