//
//  SearchLight.cpp
//  crag
//
//  Created by John on 2014-05-27.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SearchLight.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::SearchLight definitions

SearchLight::SearchLight(
	LeafNode::Init const & init, Transformation const & local_transformation, 
	Color4f const & color, LightType type, Vector2 angle, ObjectHandle exception)
: Light(init, local_transformation, color, type, exception)
, _angle(angle)
{
	ASSERT(type == LightType::search || type == LightType::search_shadow);
	
	CRAG_VERIFY(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(SearchLight, object)
	CRAG_VERIFY(static_cast<Light const &>(object));

	CRAG_VERIFY(object._angle);
	CRAG_VERIFY_UNIT(object._angle, .001f);
	CRAG_VERIFY_OP(object._angle.x, > , 0.f);
	CRAG_VERIFY_OP(object._angle.x, <= , 1.f);
	CRAG_VERIFY_OP(object._angle.y, >= , 0.f);
	CRAG_VERIFY_OP(object._angle.y, < , 1.f);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void SearchLight::SetAngle(Vector2 angle)
{
	_angle = angle;
	CRAG_VERIFY(* this);
}

Vector2 SearchLight::GetAngle() const
{
	CRAG_VERIFY(* this);
	return _angle;
}
