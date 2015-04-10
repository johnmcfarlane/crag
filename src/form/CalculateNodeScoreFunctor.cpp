//
//  CalculateNodeScoreFunctor.cpp
//  crag
//
//  Created by John on 7/5/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "CalculateNodeScoreFunctor.h"

#include "form/Node.h"

#include "core/ConfigEntry.h"


using namespace form;

CONFIG_DEFINE(node_score_recalc_coefficient, .1);
CONFIG_DEFINE(node_score_score_coefficient, 1.5);

namespace
{
	gfx::LodParameters invalid_lod_parameters = 
	{
		Vector3::Max(),
		-1.f
	};
}

////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreFunctor definitions

CalculateNodeScoreFunctor::CalculateNodeScoreFunctor()
{
	// make sure that the initial position is just plain wrong!
	SetLodParameters(GetInvalidLodParameters());
}

gfx::LodParameters const & CalculateNodeScoreFunctor::GetInvalidLodParameters()
{
	return invalid_lod_parameters;
}

bool CalculateNodeScoreFunctor::IsSignificantlyDifferent(Vector3 const & other_lod_center) const
{
	Scalar distance_squared = DistanceSq(other_lod_center, _lod_parameters.center);
	return distance_squared >= min_recalc_distance_squared;
}

void CalculateNodeScoreFunctor::ResetCounters()
{
	min_leaf_distance_squared = std::numeric_limits<Scalar>::max();
	leaf_score_range[0] = std::numeric_limits<float>::max();
	leaf_score_range[1] = std::numeric_limits<float>::min();
}

geom::Vector2f CalculateNodeScoreFunctor::GetLeafScoreRange() const
{
	return leaf_score_range;
}

Scalar CalculateNodeScoreFunctor::GetMinLeafDistanceSquared() const
{
	return min_leaf_distance_squared;
}

void CalculateNodeScoreFunctor::SetLodParameters(gfx::LodParameters const & lod_parameters)
{
	CRAG_VERIFY(lod_parameters);

	_lod_parameters = lod_parameters;

	min_recalc_distance_squared = Scalar(Squared(node_score_recalc_coefficient * lod_parameters.min_distance));
	double min_score_distance_squared_precise = Squared(node_score_score_coefficient * lod_parameters.min_distance);
	min_score_distance_squared = Scalar(min_score_distance_squared_precise);
	inverse_min_score_distance_squared = Scalar(1. / min_score_distance_squared_precise);
}

void CalculateNodeScoreFunctor::operator()(Node & node)
{
	ASSERT(node.IsInUse());
	
	float score = node.area;
	
	// distance	
	geom::Vector3f node_to_lod_center = _lod_parameters.center - node.center;
	float distance_squared = MagnitudeSq(node_to_lod_center);
	ASSERT(distance_squared < std::numeric_limits<float>::max());
	if (distance_squared > 0) 
	{
		node_to_lod_center *= FastInvSqrt(distance_squared);
	}
	else 
	{
		node_to_lod_center = geom::Vector3f(1,0,0);
	}
	ASSERT(NearEqual(MagnitudeSq(node_to_lod_center), 1.f, 1.02f));
	
	// towardness: -1=facing away, 1=facing towards
	// purpose: favour polys which are facing towards the LOD center
	float lod_center_dp = DotProduct(node_to_lod_center, node.normal);
	float towardness_factor = std::exp(lod_center_dp);
	score *= towardness_factor;
	
	// Distance-based falloff.
	if (distance_squared > min_score_distance_squared)
	{
		score /= distance_squared;
	}
	else 
	{
		score *= inverse_min_score_distance_squared;
	}
	
	ASSERT(score >= 0);
	node.score = score;
	
	if (node.IsLeaf())
	{
		if (score < leaf_score_range.x)
		{
			leaf_score_range.x = score;
		}
		if (score > leaf_score_range.y)
		{
			leaf_score_range.y = score;
		}
		
		if (distance_squared < min_leaf_distance_squared)
		{
			min_leaf_distance_squared = distance_squared;
		}
	}
}
