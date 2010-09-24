/*
 *  CalculateNodeScoreFunctor.h
 *  Crag
 *
 *  Created by John on 3/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/node/NodeFunctor.h"

#include "form/node/Node.h"

#include "core/floatOps.h"
#include "geom/Vector3.h"


namespace form 
{

	// A functor for use primarily in NodeBuffer::ForEachNode_Paralell/Serial.
	// An object of this class is a member of NodeBuffer and stores all data needed to score the buffer's nodes.
	
	class CalculateNodeScoreFunctor : public NodeFunctor
	{
		OBJECT_NO_COPY (CalculateNodeScoreFunctor);
		
	public:
		
		CalculateNodeScoreFunctor();

		// Returns a ray that is significantly different to any valid ray. 
		static Ray3 GetInvalidRay();
		
		// Returns true iff using the functor with the given ray instead
		// would yield significantly different scores.
		bool IsSignificantlyDifferent(Ray3 const & other_camera_ray) const;
		
		void SetCameraRay(Ray3 const & new_camera_ray);

		void operator()(form::Node & node) const;

	private:
		Ray3 camera_ray;
		Scalar min_recalc_distance_squared;
		Scalar min_score_distance_squared;
		Scalar inverse_min_score_distance_squared;
	};

}	// form
