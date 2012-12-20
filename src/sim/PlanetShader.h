//
//  PlanetShader.h
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/axes.h"

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
		
		void CalcRootPointPos(Random & rnd, axes::VectorAbs & position) const;
		axes::ScalarAbs GetRandomHeightCoefficient(Random & rnd) const;

		// Mid-Point Calculation 
		class Params;
		bool CalcMidPointPos_Random(form::Polyhedron & polyhedron, axes::VectorAbs & result, Params & params) const;
		bool CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, axes::VectorAbs & result, Params & params) const;

		axes::VectorAbs GetLocalPosition(form::Point const & point, axes::VectorAbs const & center) const;
		axes::VectorAbs GetLocalPosition(form::Vector3 const & point_pos, axes::VectorAbs const & center) const;
		axes::ScalarAbs GetAltitude(form::Point const & point, axes::VectorAbs const & center) const;
		axes::ScalarAbs GetAltitude(form::Vector3 const & point_pos, axes::VectorAbs const & center) const;
		axes::ScalarAbs GetAltitude(axes::VectorAbs const & local_pos) const;
	};
}
