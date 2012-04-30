//
//  Vector.h
//  Planets
//
//  Created by John on May 18, 2010.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom 
{
	// This is the un-specialized version of the Vector class.
	// Vector classes mostly store and calculate geometry. A few notes:
	// - The Vector classes are Vector2, Vector3 and Vector4.
	// - The classes are kept to a minimum; Most operators (e.g. operator+) and other functions (e.g. LengthSq) are global.
	template<typename S, int N> class Vector
	{
	};
}
