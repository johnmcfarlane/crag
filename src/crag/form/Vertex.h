/*
 *  Vertex.h
 *  Crag
 *
 *  Created by john on 5/1/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */
 
#pragma once

#include "core/Vector3.h"


namespace form
{

	class Vertex
	{
	public:
		// Attributes
		Vector3f pos;

		// Colour
		//typedef unsigned char ColorComp;
		//ColorComp red;
		//ColorComp green;
		//ColorComp blue;
		//ColorComp flags;	// currently unused

		Vector3f norm;

		Vector2f texture;
	};
}
