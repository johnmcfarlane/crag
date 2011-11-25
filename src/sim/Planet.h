/*
 *  sim/Planet.h
 *  Crag
 *
 *  Created by john on 4/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Entity.h"

#include "gfx/defs.h"


namespace form
{
	class Formation;
}


namespace sim
{
	class PlanetaryBody;
	
	
	// A planet is a celestial body which is represented using formation.
	class Planet : public Entity
	{
		DECLARE_SCRIPT_CLASS(Planet, Entity);
		
	public:
		Planet();
		~Planet();

		// Type-specific allocation via script.
		static void Create(Planet & planet, PyObject & args);

		// Called from the simulation thread.
		virtual bool Init(Simulation & simulation, PyObject & args);

		virtual void Tick(Simulation & simulation);
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual void UpdateModels() const;
		
		Scalar GetRadiusMean() const;
		Scalar GetRadiusMin() const;
		Scalar GetRadiusMax() const;
		
		void SetRadiusMinMax(Scalar radius_min, Scalar radius_max);
		
		form::Formation const & GetFormation() const;
		virtual Vector3 const & GetPosition() const;

	private:
		form::Formation * _formation;
		PlanetaryBody * _body;
		gfx::Uid _model_uid;
		Scalar _radius_mean;
		Scalar _radius_min;
		Scalar _radius_max;
	};
}
