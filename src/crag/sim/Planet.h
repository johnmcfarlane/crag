/*
 *  Planet.h
 *  Crag
 *
 *  Created by john on 4/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Entity.h"

#include "PlanetaryBody.h"

#include "glpp/Texture.h"


namespace form
{
	class Formation;
	class ShaderFactory;
}


namespace sim
{
	// A planet is a celestial body which is represented using formation.
	// TODO: The moon is of class, planet. As such, Planet needs a better name. 
	class Planet : public Entity
	{
	public:
		Planet(int num_craters, int init_seed, sim::Vector3 const & init_pos, Scalar init_radius_medium, Scalar init_radius_range);
		~Planet();

		virtual void GetGravitationalForce(sim::Vector3 const & pos, sim::Vector3 & gravity) const;
		virtual bool GetRenderRange(Ray3 const & camera_ray, Scalar * range, bool wireframe) const;
	
		Scalar GetRadiusMin() const { return radius_min; }
		Scalar GetRadiusMax() const { return radius_max; }
		Scalar GetRadiusAverage() const;
		form::Formation const & GetFormation() const;
		sim::Vector3 const & GetPosition() const;

		//DUMP_OPERATOR_DECLARATION(Entity);

	private:
		form::ShaderFactory * factory;
		form::Formation * formation;
		PlanetaryBody * body;
		Scalar radius_min, radius_max;
	};
}
