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
	// TODO: Once this code it more mature, re-merge it with the moon code.
	class PlanetShader : public form::Shader
	{
	public:
		PlanetShader(Planet const & init_planet);
		
	private:
		virtual void SetOrigin(Vector3d const & origin);
		virtual void InitRootPoints(form::Point * points[]);
		virtual bool InitMidPoint(form::Point & mid_point, form::Node const & a, form::Node const & b, int index);
		
		void CalcRootPointPos(Random & rnd, Vector3 & position) const;

		// Mid-Point Calculation 
		class Params;
		bool CalcMidPointPos_Random(sim::Vector3 & result, Params & params) const;
		bool CalcMidPointPos_SimpleInterp(sim::Vector3 & result, Params & params) const;
		bool CalcMidPointPos_BicubicInterp(sim::Vector3 & result, Params & params) const;

		typedef form::Point const * PointGrid [4][4];
		typedef form::Node const * NodeLattice [3][3][2];
		
		static bool GetPointGrid(PointGrid & grid, Params const & params);
		static void GetNodeLattice(NodeLattice & lattice, Params const & params);
		static bool LatticeToGrid(PointGrid & grid, NodeLattice const & lattice, int index);
		void GridToAltitude(Scalar altitude[4][4], PointGrid const & grid) const;
		
		Vector3 GetLocalPosition(form::Point const & point) const;
		Vector3 GetLocalPosition(form::Vector3 const & point_pos) const;
		Scalar GetAltitude(form::Point const & point) const;
		Scalar GetAltitude(form::Vector3 const & point_pos) const;
		Scalar GetAltitude(Vector3 const & local_pos) const;
		
		Vector3 center;		// relative to origin
		Planet const & planet;
	};

	
	// The factory for making PlanetShader objects.
	// Each object will be assigned to a specific polyhedron.
	class PlanetShaderFactory : public form::ShaderFactory
	{
	public:
		PlanetShaderFactory(Planet const & init_planet);
		
		virtual form::Shader * Create(form::Formation const & formation) const;
		
	private:
		Planet const & planet;	// This system is very complicated for one type of formation (Planet) and only one planet!
	};
}
