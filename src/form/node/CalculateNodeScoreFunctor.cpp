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

#include "form/node/Node.h"

#include "core/ConfigEntry.h"


using namespace form;


extern float camera_near;


CONFIG_DEFINE(node_score_recalc_coefficient, double, 0.25);
CONFIG_DEFINE(node_score_score_coefficient, double, 1.5);


////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreFunctor definitions

CalculateNodeScoreFunctor::CalculateNodeScoreFunctor()
{
	// make sure that the initial ray is just plain wrong!
	camera_ray = GetInvalidRay();
	
	// Initialize all cached values used by the functor. Try and maintain a high degree of precision. 
	min_recalc_distance_squared = Scalar(Square(node_score_recalc_coefficient * camera_near));
	double min_score_distance_squared_precise = Square(node_score_score_coefficient * camera_near);
	min_score_distance_squared = Scalar(min_score_distance_squared_precise);
	inverse_min_score_distance_squared = Scalar(1. / min_score_distance_squared_precise);
}

Ray3 CalculateNodeScoreFunctor::GetInvalidRay()
{
	return Ray3(Vector3::Max(), Vector3::Max());
}

bool CalculateNodeScoreFunctor::IsSignificantlyDifferent(Ray3 const & other_camera_ray) const
{
	Scalar distance_squared = DistanceSq(other_camera_ray.position, camera_ray.position);
	return distance_squared >= min_recalc_distance_squared;
}

void CalculateNodeScoreFunctor::ResetLeafScoreRange()
{
	leaf_score_range[0] = std::numeric_limits<float>::max();
	leaf_score_range[1] = std::numeric_limits<float>::min();
}

geom::Vector2f CalculateNodeScoreFunctor::GetLeafScoreRange() const
{
	return leaf_score_range;
}

void CalculateNodeScoreFunctor::SetCameraRay(Ray3 const & new_camera_ray)
{
	camera_ray = new_camera_ray;
}

void CalculateNodeScoreFunctor::operator()(Node & node)
{
	ASSERT(node.IsInUse());
	
	float score = node.area;
	
	// distance	
	geom::Vector3f node_to_camera = camera_ray.position - node.center;
	float distance_squared = LengthSq(node_to_camera);
	ASSERT(distance_squared < std::numeric_limits<float>::max());
	if (distance_squared > 0) 
	{
		node_to_camera *= FastInvSqrt(distance_squared);
	}
	else 
	{
		node_to_camera = geom::Vector3f(1,0,0);
	}
	ASSERT(NearEqual(LengthSq(node_to_camera), 1.f, 1.02f));
	
	// towardness: -1=facing away, 1=facing towards
	// purpose: favour polys which are facing towards the camera
	float camera_dp = DotProduct(node_to_camera, node.normal);
	float towardness_factor = std::exp(camera_dp);
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
	}
}
