//
//  PlanetShader.h
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"
#include "form/Shader.h"

#include "core/Random.h"

#include "geom/Space.h"

namespace sim
{
	// forward declarations
	class Planet;
	
	// A class which governs how the edges of individual polys of a planet's surface are sub-divided.
	class PlanetShader : public form::Shader
	{
	private:
		void InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const override;
		bool InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const override;
		
		void CalcRootPointPos(Random & rnd, geom::uni::Vector3 & position) const;
		geom::uni::Scalar GetRandomHeightCoefficient(Random & rnd) const;

		// Mid-Point Calculation 
		struct Params;
		bool CalcMidPointPos_Random(form::Polyhedron & polyhedron, geom::uni::Vector3 & result, Params & params) const;
		bool CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, geom::uni::Vector3 & result, Params & params) const;

		geom::uni::Vector3 GetLocalPosition(form::Point const & point, geom::uni::Vector3 const & center) const;
		geom::uni::Vector3 GetLocalPosition(form::Vector3 const & point_pos, geom::uni::Vector3 const & center) const;
		geom::uni::Scalar GetAltitude(form::Point const & point, geom::uni::Vector3 const & center) const;
		geom::uni::Scalar GetAltitude(form::Vector3 const & point_pos, geom::uni::Vector3 const & center) const;
		geom::uni::Scalar GetAltitude(geom::uni::Vector3 const & local_pos) const;
	};
}
