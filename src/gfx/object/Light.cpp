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
	LeafNode::Init const & init, Transformation const & local_transformation, 
	Color4f const & color, LightType type, 
	ObjectHandle exception, Scalar angle)
: LeafNode(init, local_transformation, Layer::light)
, _color(color)
, _angle(std::sqrt(.5f), std::sqrt(.5f))
, _exception(nullptr)
, _type(type)
{
	SetAngle(angle);

	auto & engine = GetEngine();
	if (exception)
	{
		auto exception_object = engine.GetObject(exception.GetUid());
		ASSERT(exception_object);
		
		if (exception_object)
		{
			_exception = & core::StaticCast<LeafNode>(* exception_object);
			CRAG_DEBUG_DUMP(_exception);
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
	CRAG_VERIFY(static_cast<Light::super const &>(object));

	CRAG_VERIFY_TRUE(Light::List::is_contained(object));
	CRAG_VERIFY_TRUE(object._color.a == 1.f);
	CRAG_VERIFY_UNIT(object._angle, .0001f);
	CRAG_VERIFY_OP(object._angle.x, >= , 0.f);
	CRAG_VERIFY_OP(object._angle.y, >= , 0.f);
	CRAG_VERIFY_TRUE(int(object._type) >= 0);
	CRAG_VERIFY_TRUE(object._type < LightType::size);
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

void Light::SetAngle(Scalar angle)
{
	_angle.x = std::sin(angle);
	_angle.y = std::cos(angle);
}

Vector2 Light::GetAngleVector() const
{
	CRAG_VERIFY(* this);
	return _angle;
}

LightType Light::GetType() const
{
	return _type;
}

bool Light::MakesShadow() const
{
	switch (_type)
	{
		case LightType::point_shadow:
		case LightType::search_shadow:
			return true;
			
		default:
			return false;
	}
}

LeafNode const * Light::GetException() const
{
	return _exception;
}

#if defined(CRAG_GFX_LIGHT_DEBUG)
LeafNode::PreRenderResult Light::PreRender()
{
	Vector3 intensity(_color.r, _color.g, _color.b);
	Debug::AddBasis(GetModelTransformation(), geom::Length(intensity));
	
	return ok;
}
#endif
