//
//  Planet.h
//  crag
//
//  Created by john on 4/13/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"

#include "geom/Sphere.h"


namespace form
{
	class Formation;
}

namespace gfx
{
	class Planet;
}


namespace sim
{
	class PlanetaryBody;
	
	// A planet is a celestial body which is represented using the formation system.
	class Planet : public Entity
	{
	public:
		Planet();
		~Planet();

		// Called from the simulation thread.
		virtual void Init(Simulation & simulation, InitData<Planet> const & init_data);

		virtual void Tick(Simulation & simulation);
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual void UpdateModels() const;
		
		Scalar GetRadiusMean() const;
		Scalar GetRadiusMin() const;
		Scalar GetRadiusMax() const;
		
		void SetRadiusMinMax(Scalar radius_min, Scalar radius_max);
		
		form::Formation const & GetFormation() const;

	private:
		form::Formation * _formation;
		PlanetaryBody * _body;
		gfx::BranchNodeHandle _branch_node;
		smp::Handle<gfx::Planet> _model;
		Scalar _radius_mean;
		Scalar _radius_min;
		Scalar _radius_max;
	};

	// InitData struct specialization for Box
	template <>
	struct InitData<Planet>
	{
		Sphere3 sphere;
		int random_seed;
		int num_craters;
	};
}
