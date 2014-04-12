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

#include "gfx/LodParameters.h"

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

		// Returns a lod center that is significantly different to any valid position. 
		static gfx::LodParameters const & GetInvalidLodParameters();
		
		// Returns true iff using the functor with the given lod center instead
		// would yield significantly different scores.
		bool IsSignificantlyDifferent(Vector3 const & other_lod_center) const;
		
		void ResetCounters();
		geom::Vector2f GetLeafScoreRange() const;
		Scalar GetMinLeafDistanceSquared() const;
		
		void SetLodParameters(gfx::LodParameters const & lod_parameters);

		void operator() (Node & node);

	private:
		gfx::LodParameters _lod_parameters;
		Scalar min_recalc_distance_squared;
		Scalar min_score_distance_squared;
		Scalar inverse_min_score_distance_squared;
		Scalar min_leaf_distance_squared;
		
		geom::Vector2f leaf_score_range;
	};

}	// form
