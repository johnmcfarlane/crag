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


namespace sim
{
	// forward declarations
	class Planet;
	
	
	// A class which governs how the edges of individual polys of a planet's surface are sub-divided.
	class PlanetShader : public form::Shader
	{
	public:
		PlanetShader(Planet const & init_planet);
		
		typedef sim::Scalar S;
		typedef sim::Vector3 Vec3;
		
	private:
		virtual void SetOrigin(Vector3d const & origin);
		virtual void InitRootPoints(int seed, form::Point * points[]);
		virtual void InitMidPoint(int i, form::Node const & a, form::Node const & b, form::Point & mid_point);
		
		Vector3f CalcMidPointPos_Shallow(int seed, Vector3f const & near_corner1, Vector3f const & near_corner2);
		Vector3f CalcMidPointPos_Medium(int seed, Vector3f const & near_corner1, Vector3f const & near_corner2, Vector3f const & far_corner1, Vector3f const & far_corner2);
		Vector3f CalcMidPointPos_Deep(int seed, Vector3f const & near_corner1, Vector3f const & near_corner2, Vector3f const & far_corner1, Vector3f const & far_corner2);
		
		Vec3 center;		// relative to origin
		Planet const & planet;
	};

	class PlanetShaderFactory : public form::ShaderFactory
	{
	public:
		PlanetShaderFactory(Planet const & init_planet);
		
		virtual form::Shader * Create(form::Formation const & formation) const;
		
	private:
		Planet const & planet;	// This system is very complicated for one type of formation (Planet) and only one planet!
	};
}
