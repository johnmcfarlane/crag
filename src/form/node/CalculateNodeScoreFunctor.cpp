/*
 *  CalculateNodeScoreFunctor.cpp
 *  crag
 *
 *  Created by John on 7/5/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "CalculateNodeScoreFunctor.h"

#include "form/node/Node.h"

#include "core/ConfigEntry.h"


extern float camera_near;


CONFIG_DEFINE(node_score_recalc_coefficient, double, 0.25);
CONFIG_DEFINE(node_score_score_coefficient, double, 1.5);


////////////////////////////////////////////////////////////////////////////////
// form::CalculateNodeScoreFunctor definitions

form::CalculateNodeScoreFunctor::CalculateNodeScoreFunctor()
{
	// make sure that the initial ray is just plain wrong!
	camera_ray = GetInvalidRay();
	
	// Initialize all cached values used by the functor. Try and maintain a high degree of precision. 
	min_recalc_distance_squared = static_cast<Scalar>(Square(node_score_recalc_coefficient * camera_near));
	double min_score_distance_squared_precise = Square(node_score_score_coefficient * camera_near);
	min_score_distance_squared = static_cast<Scalar>(min_score_distance_squared_precise);
	inverse_min_score_distance_squared = static_cast<Scalar>(1. / min_score_distance_squared_precise);
}

form::Ray3 form::CalculateNodeScoreFunctor::GetInvalidRay()
{
	return Ray3(Vector3::Max(), Vector3::Max());
}

bool form::CalculateNodeScoreFunctor::IsSignificantlyDifferent(Ray3 const & other_camera_ray) const
{
	Scalar distance_squared = LengthSq(other_camera_ray.position - camera_ray.position);
	return distance_squared >= min_recalc_distance_squared;
}

void form::CalculateNodeScoreFunctor::SetCameraRay(Ray3 const & new_camera_ray)
{
	camera_ray = new_camera_ray;
}

void form::CalculateNodeScoreFunctor::operator()(form::Node & node) const
{
	Assert(node.IsInUse());
	
	float score = node.area;
	
	// distance	
	Vector3f node_to_camera = camera_ray.position - node.center;
	float distance_squared = LengthSq(node_to_camera);
	Assert(distance_squared < std::numeric_limits<float>::max());
	if (distance_squared > 0) 
	{
		node_to_camera *= FastInvSqrt(distance_squared);
	}
	else 
	{
		node_to_camera = Vector3f(1,0,0);
	}
	Assert(NearEqual(LengthSq(node_to_camera), 1.f, 1.02f));
	
	// towardness: -1=facing away, 1=facing towards
	// purpose: favour polys which are facing towards the camera
	float camera_dp = DotProduct(node_to_camera, node.normal);
	float towardness_factor = Exp(camera_dp);
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
	
	Assert(score >= 0);
	node.score = score;
}
