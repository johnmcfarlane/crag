//
//  Point.h
//  crag
//
//  Created by John on 5/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/LitVertex.h"

namespace form
{
	// This class stores the position of corners/mid-points in Nodes.
	// They also remember their vertex in the currently generated mesh.
	
	class Point
	{
	public:
		// types
		typedef gfx::LitVertex Vertex;
		typedef Vertex::Vector3 Vector3;
		
		// functions
		Point();

		CRAG_VERIFY_INVARIANTS_DECLARE(Point);
		
		// attributes
		Vertex::Vector3 pos;
		Vertex * vert;
	};
}
