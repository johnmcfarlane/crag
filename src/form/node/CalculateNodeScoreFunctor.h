//
//  CalculateNodeScoreFunctor.h
//  crag
//
//  Created by John on 3/28/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"

namespace form 
{
	// forward-declarations
	class Node;

	// A functor for use primarily in NodeBuffer::ForEachNode_Paralell/Serial.
	// An object of this class is a member of NodeBuffer and stores all data needed to score the buffer's nodes.
	// TODO: It doesn't make sense for this to persist across multiple Tick calls.
	class CalculateNodeScoreFunctor
	{
		OBJECT_NO_COPY (CalculateNodeScoreFunctor);
		
	public:
		
		CalculateNodeScoreFunctor();

		// Returns a ray that is significantly different to any valid ray. 
		static Ray3 GetInvalidRay();
		
		// Returns true iff using the functor with the given ray instead
		// would yield significantly different scores.
		bool IsSignificantlyDifferent(Ray3 const & other_camera_ray) const;
		
		void ResetCounters();
		geom::Vector2f GetLeafScoreRange() const;
		Scalar GetMinLeafDistanceSquared() const;
		
		void SetCameraRay(Ray3 const & new_camera_ray);

		void operator() (Node & node);

	private:
		Ray3 camera_ray;
		Scalar min_recalc_distance_squared;
		Scalar min_score_distance_squared;
		Scalar inverse_min_score_distance_squared;
		Scalar min_leaf_distance_squared;
		
		geom::Vector2f leaf_score_range;
	};

}	// form
