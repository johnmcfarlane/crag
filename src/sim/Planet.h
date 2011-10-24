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
		
		Scalar GetRadiusMean() const { return _radius_mean; }
		Scalar GetRadiusMin() const { return _radius_min; }
		Scalar GetRadiusMax() const { return _radius_max; }
		void SampleRadius(Scalar r);
		
		form::Formation const & GetFormation() const;
		virtual Vector3 const & GetPosition() const;

		//DUMP_OPERATOR_DECLARATION(Entity);

	private:
		form::Formation * _formation;
		PlanetaryBody * _body;
		gfx::Planet * _model;
		Scalar _radius_mean;
		Scalar _radius_min, _radius_max;
	};
}
