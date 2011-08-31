//
//  Planet.cpp
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet(Vector const & position)
{
	_salient._position = position;
}
		
void Planet::Update(UpdateParams const & params)
{
	_salient = params;
}

bool Planet::GetRenderRange(Ray const & camera_ray, double * range, bool wireframe) const 
{
	Scalar distance_squared = LengthSq(camera_ray.position - _salient._position);
	Scalar distance = Sqrt(distance_squared);
	
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
	Scalar a = Sqrt(Square(distance) - Square(_salient._radius_min));
	Scalar b = Sqrt(Square(_salient._radius_max) - Square(_salient._radius_min));
	range[1] = a + b;
	
	return true;
}

void Planet::Render(Layer::type layer, gfx::Scene const & scene) const 
{ 
	// actual drawing is taken care of by the formation manager
}

bool Planet::IsInLayer(Layer::type layer) const 
{ 
	return layer == Layer::foreground; 
}
