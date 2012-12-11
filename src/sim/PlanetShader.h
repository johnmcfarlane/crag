//
//  PlanetShader.h
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"

#include "form/defs.h"
#include "form/node/Shader.h"

#include "core/Random.h"

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
		
		void CalcRootPointPos(Random & rnd, Vector3 & position) const;
		Scalar GetRandomHeightCoefficient(Random & rnd) const;

		// Mid-Point Calculation 
		class Params;
		bool CalcMidPointPos_Random(form::Polyhedron & polyhedron, Vector3 & result, Params & params) const;
		bool CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, Vector3 & result, Params & params) const;

		Vector3 GetLocalPosition(form::Point const & point, Vector3 const & center) const;
		Vector3 GetLocalPosition(form::Vector3 const & point_pos, Vector3 const & center) const;
		Scalar GetAltitude(form::Point const & point, Vector3 const & center) const;
		Scalar GetAltitude(form::Vector3 const & point_pos, Vector3 const & center) const;
		Scalar GetAltitude(Vector3 const & local_pos) const;
	};
}
