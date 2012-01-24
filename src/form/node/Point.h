//
//  Point.h
//  crag
//
//  Created by John on 5/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"

#include "form/Scene/Vertex.h"


namespace form
{
	// This class stores the position of corners/mid-points in Nodes.
	// They also remember their vertex in the currently generated mesh.
	
	class Point
	{
	public:
		Point();
		
		// Attributes
		Vector3 pos;
		Vertex * vert;
	};
}
