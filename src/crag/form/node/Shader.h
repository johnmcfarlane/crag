/*
 *  Shader.h
 *  Crag
 *
 *  Created by John on 2/21/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/defs.h"


namespace form
{
	class Node;
	class Point;
	
	// As with most things called shader, has absolutely FA to do with shade.
	// Tesselates a shape given two/four surrounding points.
	// Instance per form::Formation per form::Scene.
	// TODO: tidy up interface / fns.
	class Shader
	{
	public:	
		virtual ~Shader() { }
		
		virtual void SetOrigin(Vector3d const & origin) = 0;
		virtual void InitRootPoints(int seed, form::Point * points[]) = 0;
		virtual void InitMidPoint(int i, Node const & a, Node const & b, Point & mid_point) = 0;
	};
	
	// TODO: This pattern is probably over-complicated.
	class ShaderFactory
	{
	public:
		virtual ~ShaderFactory() { }
		
		virtual Shader * Create(class Formation const & formation) const = 0;
	};
}
