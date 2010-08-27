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


void form::CalculateNodeScoreFunctor::operator()(form::Node & node) const
{
	Assert(node.IsInUse());
	
	float score = node.area;
	
	// distance
	Vector3f camera_to_node = node.center - relative_camera_pos;
	float distance_squared = LengthSq(camera_to_node);
	Assert(distance_squared < std::numeric_limits<float>::max());
	if (distance_squared > 0) 
	{
		camera_to_node *= FastInvSqrt(distance_squared);
	}
	else 
	{
		camera_to_node = Vector3f(1,0,0);
	}
	Assert(NearEqual(LengthSq(camera_to_node), 1.f, 1.02f));
	
	// towardness: -1=facing away, 1=facing towards
	// purpose: favour polys which are facing towards the camera
	float camera_dp = DotProduct(camera_to_node, node.normal);
	float towardness_factor = Exp(camera_dp);
	score *= towardness_factor;

	// Distance-based falloff.
	float fudged_min_visible_distance = camera_near;	// same as the near plane (or the camera radius, whichever is greater)
	score /= Max(distance_squared, Square(fudged_min_visible_distance));
	
	Assert(score >= 0);
	node.score = score;
}
