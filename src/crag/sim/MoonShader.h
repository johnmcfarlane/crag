/*
 *  MoonShader.h
 *  crag
 *
 *  Created by John on 8/9/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "form/node/Shader.h"

#include "core/Random.h"


namespace sim
{
	// forward declarations
	class Planet;
	
	
	// A class which governs how the edges of individual polys of a moon's surface are sub-divided.
	class MoonShader : public form::Shader
	{
	public:
		MoonShader(Planet const & init_moon, int num_craters);
		
	private:
		virtual void SetOrigin(Vector3d const & origin);
		virtual void InitRootPoints(form::Point * points[]);
		virtual bool InitMidPoint(form::Point & mid_point, form::Node const & a, form::Node const & b, int index);
		
		void CalcPointPos(Vector3 & position) const;
		
		void ApplyCraters(Random rnd, Vector3 & position) const;
		void GenerateCreater(Random & rnd, Sphere3 & crater) const;
		
		Vector3 center;		// relative to origin
		Planet const & moon;
		
		typedef std::vector<Sphere3> CraterVector;
		CraterVector craters;
	};
	
	
	// The factory for making PlanetShader objects.
	// Each object will be assigned to a specific polyhedron.
	class MoonShaderFactory : public form::ShaderFactory
	{
	public:
		MoonShaderFactory(Planet const & init_moon, int num_craters);
		
		virtual form::Shader * Create(form::Formation const & formation) const;
		
	private:
		Planet const & moon;
		int num_craters;
	};
}
