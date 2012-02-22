//
//  Shader.h
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"


namespace form
{
	class Node;
	class Point;
	class Polyhedron;
	
	// As with most things called shader, has absolutely FA to do with shade.
	// Tesselates a shape given two/four surrounding points.
	// Instance per form::Formation per form::Scene.
	class Shader
	{
	public:	
		virtual ~Shader() { }
		virtual void InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const = 0;
		virtual bool InitMidPoint(Polyhedron & polyhedron, Node const & a, Node const & b, int index, Point & mid_point) const = 0;
	};
}
