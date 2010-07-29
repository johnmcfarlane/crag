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
}


namespace sim
{
	class PlanetaryBody;
	class PlanetShaderFactory;

	class Planet : public Entity
	{
	public:
		Planet(sim::Vector3 const & init_pos, float init_min_radius, float init_max_radius, int init_seed);
		~Planet();

		virtual void GetGravitationalForce(sim::Vector3 const & pos, sim::Vector3 & gravity) const;
		virtual bool GetRenderRange(Ray3 const & camera_ray, double * range) const;
	
		Scalar GetRadiusMin() const { return radius_min; }
		Scalar GetRadiusMax() const { return radius_max; }
		Scalar GetAverageRadius() const;
		sim::Vector3 const & GetPosition() const;

		//DUMP_OPERATOR_DECLARATION(Entity);

	private:
		float radius_min, radius_max;
		PlanetShaderFactory * factory;
		form::Formation * formation;
		PlanetaryBody * body;
	};
}
