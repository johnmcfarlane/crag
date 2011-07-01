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
		DECLARE_SCRIPT_CLASS(Planet, Entity);
		
	public:
		Planet();
		~Planet();

		// Type-specific allocation via script.
		static void Create(Planet & planet, PyObject & args);

		// Called from the simulation thread.
		virtual bool Init(PyObject & args);

		virtual void Tick();
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual bool GetRenderRange(Ray3 const & camera_ray, Scalar * range, bool wireframe) const;
	
		Scalar GetRadiusMean() const { return radius_mean; }
		Scalar GetRadiusMin() const { return radius_min; }
		Scalar GetRadiusMax() const { return radius_max; }
		void SampleRadius(Scalar r);
		
		form::Formation const & GetFormation() const;
		virtual Vector3 const & GetPosition() const;

		//DUMP_OPERATOR_DECLARATION(Entity);

	private:
		form::ShaderFactory * factory;
		form::Formation * formation;
		PlanetaryBody * body;
		Scalar radius_mean;
		Scalar radius_min, radius_max;
	};
}
