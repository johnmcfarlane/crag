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

#include "form/defs.h"

#include "gfx/Color.h"


namespace form
{
	// This class stores the position of corners/mid-points in Nodes.
	// They also remember their vertex in the currently generated mesh.
	
	// TODO: form/defs.f with Vector3.
	class Point
	{
	public:
		Point();
		
		// Attributes
		Vector3 pos;
		gfx::Color4b col;
		Vertex * vert;
	};
}
