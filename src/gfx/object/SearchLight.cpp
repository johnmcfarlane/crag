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

namespace
{
	LightAttributes const search_light_attribs =
	{
		LightResolution::fragment,
		LightType::search,
#if defined(CRAG_GL)
		false
#endif
#if defined(CRAG_GLES)
		false
#endif
	};
}

////////////////////////////////////////////////////////////////////////////////
// gfx::SearchLight definitions

SearchLight::SearchLight(
	Engine & engine, Transformation const & local_transformation, 
	Color4f const & color, Vector2 angle, ObjectHandle exception)
: Light(
	engine, local_transformation, color, 
	search_light_attribs, exception)
, _angle(angle)
{
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
