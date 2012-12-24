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

DECLARE_CLASS_HANDLE(gfx, BranchNode)	// gfx::BranchNodeHandle
DECLARE_CLASS_HANDLE(gfx, Planet)	// gfx::PlanetHandle


namespace form
{
	class Formation;
}


namespace sim
{
	class PlanetaryBody;
	
	// A planet is a celestial body which is represented using the formation system.
	class Planet : public Entity
	{
	public:
		// functions
		Planet(Init const & init, Sphere3 sphere, int random_seed, int num_craters);
		~Planet();

		virtual void Tick(sim::Engine & simulation_engine);
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual void UpdateModels() const;
		
		Scalar GetRadiusMean() const;
		Scalar GetRadiusMin() const;
		Scalar GetRadiusMax() const;
		
		void SetRadiusMinMax(Scalar const & radius_min, Scalar const & radius_max);
		
		form::Formation const & GetFormation() const;

	private:
		form::Formation * _formation;
		gfx::BranchNodeHandle _branch_node;
		gfx::PlanetHandle _model;
		Scalar _radius_mean;
		Scalar _radius_min;
		Scalar _radius_max;
	};
}
