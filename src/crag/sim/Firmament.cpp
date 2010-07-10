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
#include "geom/Sphere3.h"


Firmament::Firmament()
{
	DrawStarsSimple();
	//DrawStarsClevur();
}

void Firmament::DrawStarsClevur()
{
	for (int axis = 0; axis < 3; ++ axis)
	{
		int x_axis = TriMod(axis + 1);
		int y_axis = TriMod(axis + 2);
		int z_axis = axis;
		for (int pole = 0; pole < 2; ++ pole)
		{
			gfx::Image side;
			side.Create(Vector2i(box_edge_size, box_edge_size));
			side.Clear(gfx::Color4b::Black());

			Vector2i pos;
			Vector2f f_pos;
			for (pos.x = 0; pos.x < box_edge_size; ++ pos.x)
			{
				f_pos.x = ((static_cast<float>(pos.x) + .5f) / (static_cast<float>(box_edge_size) - .0)) - .5f;

				for (pos.y = 0; pos.y < box_edge_size; ++ pos.y)
				{
					f_pos.y = ((static_cast<float>(pos.y) + .5f) / (static_cast<float>(box_edge_size) - .0)) - .5f;

					double intensity = 0;
					Random random (1);

					Sphere3f star;
					float const * axes = star.center.GetAxes();
//					float & star_x = axes[x_axis];
//					float & star_y = axes[y_axis];
					float const & star_z = axes[z_axis];

					Vector3f line_direction;
					float * line_axes = line_direction.GetAxes();
					float & line_x = line_axes[x_axis];
					float & line_y = line_axes[y_axis];
					float & line_z = line_axes[z_axis];

					for (int i = num_stars; i; -- i)
					{
						star.center.x = random.GetFloatInclusive() - .5f;
						star.center.y = random.GetFloatInclusive() - .5f;
						star.center.z = random.GetFloatInclusive() - .5f;
						star.radius = Square(random.GetFloatInclusive());

						float w = star_z;
						if ((w > 0) != (pole != 0) || w == 0)
						{
							continue;
						}

						//float w_co = .5f / w;
						//Vector2f uv(star_x * w_co, star_y * w_co);

						line_x = f_pos.x;
						line_y = f_pos.y;
						line_z = (w > 0) ? .5 : -.5;
						Normalize(line_direction);

						float dp = DotProduct(star.center, line_direction);
						float a_sq = LengthSq(star.center) - Square(dp);

						intensity += (double)star.radius / (double)a_sq;
					}

					float comp = Min(1., .00002 * intensity);
					gfx::Color4f c (comp);
					side.SetPixel(pos, c);
				}
			}

			SetSide(axis, pole, side);
		}
	}
}

void Firmament::DrawStarsSimple()
{
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
				star.radius = Square(random.GetFloatInclusive());
				
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

