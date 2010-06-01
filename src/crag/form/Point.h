/*
 *  Point.h
 *  crag
 *
 *  Created by John on 5/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/Vector3.h"

namespace form
{
	class Vertex;
	
	// This class stores the position of corners/mid-points in Nodes.
	// They also remember their vertex in the currently generated mesh.
	class Point : public Vector3f
	{
	public:
		Point();
		
		// Attributes
		Vertex * vert;
	};
}
