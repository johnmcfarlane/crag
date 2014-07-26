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
		
		void CalcRootPointPos(Random & rnd, geom::abs::Vector3 & position) const;
		geom::abs::Scalar GetRandomHeightCoefficient(Random & rnd) const;

		// Mid-Point Calculation 
		class Params;
		bool CalcMidPointPos_Random(form::Polyhedron & polyhedron, geom::abs::Vector3 & result, Params & params) const;
		bool CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, geom::abs::Vector3 & result, Params & params) const;

		geom::abs::Vector3 GetLocalPosition(form::Point const & point, geom::abs::Vector3 const & center) const;
		geom::abs::Vector3 GetLocalPosition(form::Vector3 const & point_pos, geom::abs::Vector3 const & center) const;
		geom::abs::Scalar GetAltitude(form::Point const & point, geom::abs::Vector3 const & center) const;
		geom::abs::Scalar GetAltitude(form::Vector3 const & point_pos, geom::abs::Vector3 const & center) const;
		geom::abs::Scalar GetAltitude(geom::abs::Vector3 const & local_pos) const;
	};
}
