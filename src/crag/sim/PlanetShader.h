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

#include "form/Shader.h"


namespace form {
	class Formation;
}


namespace sim
{
	class PlanetShader : public form::Shader
	{
	public:
		PlanetShader(form::Formation const & init_formation);
		
		virtual void SetOrigin(Vector3d const & origin);

		virtual void InitMidPoint(int i, form::Node const & a, form::Node const & b, form::Vertex & mid_point);
		virtual Vector3f CalcMidPointPos(int seed, Vector3f const & near_corners1, Vector3f const & near_corners2, Vector3f const & far_corners1, Vector3f const & far_corners2);
		//virtual float CalcScore(form::Node const & node);
		
	private:
		//	typedef sim::Scalar S;
		//	typedef sim::Vector3 Vec3;
		typedef float S;
		typedef Vector3f Vec3;
		
		Vec3 center;		// relative to origin
		S max_radius;
		form::Formation const & formation;
	};

	class PlanetShaderFactory : public form::ShaderFactory
	{
	public:
		virtual form::Shader * Create(form::Formation const & formation) const;
	};
}
