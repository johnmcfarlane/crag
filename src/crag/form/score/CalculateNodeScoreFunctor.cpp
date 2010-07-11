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
	Vector3f to_camera = relative_camera_pos - node.center;
	float distance_squared = LengthSq(to_camera);
	Assert(distance_squared < std::numeric_limits<float>::max());
	if (distance_squared > 0) {
		to_camera *= FastInvSqrt(distance_squared);
	}
	else {
		to_camera = Vector3f(1,0,0);
	}
	Assert(NearEqual(LengthSq(to_camera), 1.f, 1.02f));
	
#if 0
	// towardness: -1=facing away, 1=facing towards
	// purpose: favour polys which are facing towards the camera
	float camera_dp = DotProduct(to_camera, node.normal);
	float towardness_factor = AngleToScoreFactor(1.0f, .01f, camera_dp);
	score *= towardness_factor;
	
#if 0
	// profileness: -1=facing away or towards, 1=facing perpendicular to camera
	// purpose: favour polys which are on the horizon
	float profileness_factor = sqrtf(1.f - Square(camera_dp)) * .5f + .5f;
	score *= profileness_factor;
#endif
#endif
	
#if 0
	// Controversial: reduces the detail behind the camera.
	float forwardness_factor = AngleToScoreFactor(.5f, 1.f, to_camera, camera_direction);
	score *= forwardness_factor;
#endif
	
	// Distance-based falloff.
	float fudged_min_visible_distance = camera_near;	// same as the near plane (or the camera radius, whichever is greater)
	score /= Max(distance_squared, Square(fudged_min_visible_distance));
	
	Assert(score >= 0);
	node.score = score;
}
