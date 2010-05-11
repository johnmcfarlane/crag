/*
 *  Firmament.cpp
 *  Crag
 *
 *  Created by John on 11/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Firmament.h"

#include "gfx/Image.h"
#include "gfx/Color.h"

#include "core/Random.h"
#include "core/Sphere3.h"


Firmament::Firmament()
{
	int num_stars = 1000;
	
	for (int axis = 0; axis < 3; ++ axis)
	{
		for (int pole = 0; pole < 2; ++ pole)
		{
			gfx::Image side;
			side.Create(Vector2i(box_edge_size, box_edge_size));
			side.Clear(gfx::Color4b::Black());
			
			Random random (1);
			for (int i = num_stars; i; -- i)
			{
				Sphere3f star;
				//random.GetGaussians(star.center.x, star.center.y);
				//random.GetGaussians(star.center.z, star.radius);
				//star.radius = Abs(star.radius);
				star.center.x = random.GetFloatInclusive() - .5f;
				star.center.y = random.GetFloatInclusive() - .5f;
				star.center.z = random.GetFloatInclusive() - .5f;
				star.radius = random.GetFloatInclusive();
				
				float const * axes = star.center.GetAxes();
				float w = axes[axis];
				if ((w > 0) != (pole != 0) || w == 0)
				{
					continue;
				}
				
				float w_co = .5f / w;
				
				Vector2f uv(axes[TriMod(axis + 1)] * w_co, axes[TriMod(axis + 2)] * w_co);
				float radius = star.radius * Abs(w_co);
				DrawStar(side, uv, radius);
			}
			
			SetSide(axis, pole, side);
		}
	}
}

void Firmament::DrawStar(gfx::Image & side, Vector2f const & uv, float r)
{
	Vector2i pos;

	pos.x = static_cast<int>((uv.x + .5f) * box_edge_size);
	if (pos.x < 0 || pos.x >= box_edge_size) {
		return;
	}
	
	pos.y = static_cast<int>((uv.y + .5f) * box_edge_size);
	if (pos.y < 0 || pos.y >= box_edge_size) {
		return;
	}
	
	float comp = Min(1.f, r);
	gfx::Color4f c (comp);
	side.SetPixel(pos, c);
}

