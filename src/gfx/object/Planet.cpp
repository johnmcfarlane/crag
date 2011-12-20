//
//  gfx/object/Planet.cpp
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"

#include "geom/Ray.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet()
: LeafNode(Layer::foreground)
{
}

void Planet::Update(UpdateParams const & params)
{
	_salient = params;
}

bool Planet::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const 
{
	// TODO: Either [0][3] or [1][3] or this transformation matrix
	// contains the z-depth but it's signed. 
	// This fn needs adjustments to make use of it.
	// It is faster to get and more accurate than distance-to-camera.
	Vector center = transformation.GetTranslation();
	Scalar distance = Length(center);
	
	// Is camera inside the planet?
	if (distance < _salient._radius_min)
	{
		range[0] = _salient._radius_min - distance;
		range[1] = distance + _salient._radius_max; 
		return true;
	}
	
	// Is camera outside the entire planet?
	if (distance > _salient._radius_max)
	{
		range[0] = distance - _salient._radius_max;
	}
	else 
	{
		// The camera is between the min and max range of planet heights 
		// so it could be right up close to stuff.
		range[0] = 0;
	}
	
	// Finally we need to calculate the furthest distance from the camera to the planet.
	
	// For wireframe mode, it's easy (and the same as when you're inside the planet.
	if (wireframe)
	{
		range[1] = distance + _salient._radius_max; 
		return true;
	}
	
	// Otherwise, the furthest you might ever be able too see of this planet
	// is a ray that skims the sphere of _radius_min
	// and then hits the sphere of _radius_max.
	// For some reason, those two distances appear to be very easy to calculate. 
	// (Famous last words.)
	Scalar a = sqrt(Square(distance) - Square(_salient._radius_min));
	Scalar b = sqrt(Square(_salient._radius_max) - Square(_salient._radius_min));
	range[1] = a + b;
	
	return true;
}

void Planet::Render() const 
{ 
	// actual drawing is taken care of by the formation manager
}
