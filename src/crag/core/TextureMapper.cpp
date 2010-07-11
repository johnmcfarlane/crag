/*
 *  TextureMapper.cpp
 *  crag
 *
 *  Created by John on 6/4/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "TextureMapper.h"


namespace 
{
	float equalateral_triangle_height = Sqrt(.75f);
	float equalateral_triangle_[3][2] = {
		{ -.5f, 0 },
		{ .5f, 0 },
		{ 0, equalateral_triangle_height }
	};
}


TextureMapper::TextureMapper(Vector2i const & texture_resolution, int triangle_number)
{
	Init (texture_resolution, triangle_number);
}

void TextureMapper::Init(Vector2i const & texture_resolution, int triangle_number)
{
	Assert(triangle_number == 0 || triangle_number == 1);
	
	origin.x = .5f * texture_resolution.x;
	origin.y = .5f * texture_resolution.y;
	up.x = - equalateral_triangle_height / texture_resolution.x;
	up.y = - equalateral_triangle_height / texture_resolution.y;
	right.x = -.5f / texture_resolution.x;
	right.y =  .5f / texture_resolution.y;
	if (triangle_number == 1)
	{
		right *= -1;
	}
}

Vector2f TextureMapper::GetMapping(Vector2f const & uv) const
{
	Vector2f trans = uv - origin;
	Vector2f r;
	r.x = trans.x * right.x + trans.y * right.y;
	r.y = trans.x * up.x + trans.y * up.y;
	return r;
}

Vector2f TextureMapper::GetCorner(int index)
{
	Assert(index >= 0 && index < 3);
	return Vector2f(equalateral_triangle_[index][0], equalateral_triangle_[index][1]);
}
