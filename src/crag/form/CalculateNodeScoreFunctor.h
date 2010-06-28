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

#include "NodeFunctor.h"

#include "Node.h"

#include "core/floatOps.h"
#include "geom/Vector3.h"


extern float camera_near;


namespace form {

class CalculateNodeScoreFunctor : public NodeFunctor
{
public:
	CalculateNodeScoreFunctor(Vector3f const & _relative_camera_pos, Vector3f const & _camera_direction)
	: relative_camera_pos(_relative_camera_pos)
	, camera_direction(_camera_direction)
	{
	}

	void operator()(form::Node & node) const
	{
		if (! node.IsInUse()) {
			Assert (node.score == 0);
			return;
		}

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

private:

	static inline float AngleToScoreFactor(float opposite_score, float equal_score, float dot_product)
	{
		float perpendicular_score = (equal_score + opposite_score) * .5f;
		float unclipped_score = perpendicular_score + dot_product * (equal_score - perpendicular_score);
		
		// passing in range as template params should mean that this little lot
		// compiles down to something more sane than it appears.
		if (opposite_score < equal_score) {
			if (opposite_score < 0.f) {
				if (unclipped_score < 0.f) {
					return 0.f;
				}
			}
			if (equal_score > 1.f) {
				if (unclipped_score > 1.f) {
					return 1.f;
				}
			}
		}
		else {
			if (equal_score < 0.f) {
				if (unclipped_score < 0.f) {
					return 0.f;
				}
			}
			if (opposite_score >= 1.f) {
				if (unclipped_score >= 1.f) {
					return 1.f;
				}
			}
		}
		
		Assert (unclipped_score >= -0.001f && unclipped_score <= 1.001f);
		return unclipped_score;
	}

	static inline float AngleToScoreFactor(float opposite_score, float equal_score, Vector3f const & a, Vector3f const & b)
	{
		return AngleToScoreFactor(opposite_score, equal_score, DotProduct(a, b));
	}

	Vector3f const & relative_camera_pos;
	Vector3f const & camera_direction;
};

}	// form
