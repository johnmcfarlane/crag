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

	void operator()(form::Node & node) const;
	
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
