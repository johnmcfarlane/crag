/*
 *  PlanetShader.h
 *  Crag
 *
 *  Created by John on 2/21/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"

#include "form/node/Shader.h"

#include "core/Random.h"


namespace sim
{
	// forward declarations
	class Planet;
	
	
	// A class which governs how the edges of individual polys of a planet's surface are sub-divided.
	class PlanetShader : public form::Shader
	{
	public:
		PlanetShader(Planet const & init_planet, int num_craters);
		
	private:
		virtual void SetOrigin(Vector3d const & origin);
		virtual void InitRootPoints(int seed, form::Point * points[]);
		virtual void InitMidPoint(int i, form::Node const & a, form::Node const & b, form::Point & mid_point);
		
		struct Params
		{
			Random & rnd;
			Vector3 near_a;
			Vector3 near_b;
			Vector3 far_a;
			Vector3 far_b;
		};
		
		//sim::Vector3 CalcRootPointPointPos(Params & params);
		
		sim::Vector3 CalcMidPointPos_Shallow(Params & params);
		sim::Vector3 CalcMidPointPos_Medium(Params & params);
		sim::Vector3 CalcMidPointPos_Deep(Params & params);
		
		Vector3 center;		// relative to origin
		Planet const & planet;
		int num_craters;
	};

	
	// The factory for making PlanetShader objects.
	// Each object will be assigned to a specific polyhedron.
	class PlanetShaderFactory : public form::ShaderFactory
	{
	public:
		PlanetShaderFactory(Planet const & init_planet, int num_craters);
		
		virtual form::Shader * Create(form::Formation const & formation) const;
		
	private:
		Planet const & planet;	// This system is very complicated for one type of formation (Planet) and only one planet!
		int num_craters;
	};
}
