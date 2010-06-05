/*
 *  TextureMapper.h
 *  crag
 *
 *  Created by John on 6/4/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

// Given a space representing half a texture (divided diagonally down its center),
// Provides mapping to coordinates for equilatteral triangle.

class TextureMapper
{
public:
	TextureMapper(Vector2i const & texture_resolution, int triangle_number);
	
	void Init(Vector2i const & texture_resolution, int triangle_number);
	
	// From texture uv space to equilateral space.
	Vector2f GetMapping(Vector2f const & uv) const;
	
	static Vector2f GetCorner(int index);
	
private:
	Vector2f origin;	// 3x2?
	
	// TODO: 2x2 matrix?
	Vector2f up;
	Vector2f right;
};
