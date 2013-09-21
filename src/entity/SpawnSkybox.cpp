//
//  SpawnEntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "gfx/Engine.h"
#include "gfx/Image.h"
#include "gfx/object/Skybox.h"

#include "core/Random.h"

namespace
{
	void DrawHolodeckSkybox(gfx::Skybox & skybox, int box_edge_size, int num_bars)
	{
		gfx::Image image;
		image.Create(geom::Vector2i(box_edge_size, box_edge_size));
		image.Clear(gfx::Color4b::Black());
		auto square_size = box_edge_size / num_bars;
		auto color = gfx::Color4f::Cyan();

		geom::Vector2i position, alt_position;
		for (position.x = 0; position.x < box_edge_size; position.x += square_size)
		{
			alt_position.x = position.x + square_size - 1;
			for (position.y = 0; position.y < box_edge_size; ++ position.y)
			{
				alt_position.y = position.y;
				image.SetPixel(position, color);
				image.SetPixel(alt_position, color);
				image.SetPixel(geom::Vector2i(position.y, position.x), color);
				image.SetPixel(geom::Vector2i(alt_position.y, alt_position.x), color);
			}
		}
	
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				skybox.SetSide(axis, pole, image);
			}
		}
	}

	void DrawStarsSlow(gfx::Skybox & skybox, int box_edge_size, int num_stars)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			int x_axis = TriMod(axis + 1);
			int y_axis = TriMod(axis + 2);
			int z_axis = axis;
			for (int pole = 0; pole < 2; ++ pole)
			{
				gfx::Image side;
				side.Create(geom::Vector2i(box_edge_size, box_edge_size));
				side.Clear(gfx::Color4b::Black());

				geom::Vector2i pos;
				geom::Vector2f f_pos;
				for (pos.x = 0; pos.x < box_edge_size; ++ pos.x)
				{
					f_pos.x = ((static_cast<float>(pos.x) + .5f) / (static_cast<float>(box_edge_size) - .0f)) - .5f;

					for (pos.y = 0; pos.y < box_edge_size; ++ pos.y)
					{
						f_pos.y = ((static_cast<float>(pos.y) + .5f) / (static_cast<float>(box_edge_size) - .0f)) - .5f;

						double intensity = 0;
						Random random (1);

						geom::Sphere<float, 3> star;
						float const * axes = star.center.GetAxes();
						//float & star_x = axes[x_axis];
						//float & star_y = axes[y_axis];
						float const & star_z = axes[z_axis];

						geom::Vector3f line_direction;
						float * line_axes = line_direction.GetAxes();
						float & line_x = line_axes[x_axis];
						float & line_y = line_axes[y_axis];
						float & line_z = line_axes[z_axis];

						for (int i = num_stars; i; -- i)
						{
							star.center.x = random.GetUnitInclusive<float>() - .5f;
							star.center.y = random.GetUnitInclusive<float>() - .5f;
							star.center.z = random.GetUnitInclusive<float>() - .5f;
							star.radius = Squared(random.GetUnitInclusive<float>());

							float w = star_z;
							if ((w > 0) != (pole != 0) || w == 0)
							{
								continue;
							}

							//float w_co = .5f / w;
							//geom::Vector2f uv(star_x * w_co, star_y * w_co);

							line_x = f_pos.x;
							line_y = f_pos.y;
							line_z = (w > 0) ? .5f : -.5f;
							Normalize(line_direction);

							float dp = DotProduct(star.center, line_direction);
							float a_sq = LengthSq(star.center) - Squared(dp);

							intensity += (double)star.radius / (double)a_sq;
						}

						float comp = static_cast<float>(std::min(1., .0002 * intensity));
						gfx::Color4f c (comp);
						side.SetPixel(pos, c);
					}
				}

				skybox.SetSide(axis, pole, side);
			}
		}
	}

	void DrawStar(gfx::Image & side, geom::Vector2f const & uv, float r)
	{
		auto size = side.GetSize();
		geom::Vector2i pos;

		pos.x = static_cast<int>((uv.x + .5f) * size.x);
		if (pos.x < 0 || pos.x >= size.x) {
			return;
		}
	
		pos.y = static_cast<int>((uv.y + .5f) * size.y);
		if (pos.y < 0 || pos.y >= size.y) {
			return;
		}
	
		float comp = std::min(1.f, r);
		gfx::Color4f c (comp);
		side.SetPixel(pos, c);
	}

	void DrawStarsFast(gfx::Skybox & skybox, int box_edge_size, int num_stars)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				gfx::Image side;
				side.Create(geom::Vector2i(box_edge_size, box_edge_size));
				side.Clear(gfx::Color4b::Black());
			
				Random random (1);
				for (int i = num_stars; i; -- i)
				{
					geom::Sphere<float, 3> star;
					//random.GetGaussians(star.center.x, star.center.y);
					//random.GetGaussians(star.center.z, star.radius);
					//star.radius = Abs(star.radius);
					star.center.x = random.GetUnitInclusive<float>() - .5f;
					star.center.y = random.GetUnitInclusive<float>() - .5f;
					star.center.z = random.GetUnitInclusive<float>() - .5f;
					star.radius = Squared(random.GetUnitInclusive<float>());
				
					float const * axes = star.center.GetAxes();
					float w = axes[axis];
					if ((w > 0) != (pole != 0) || w == 0)
					{
						continue;
					}
				
					float w_co = .5f / w;
				
					geom::Vector2f uv(axes[TriMod(axis + 1)] * w_co, axes[TriMod(axis + 2)] * w_co);
					float radius = star.radius * Abs(w_co);
					DrawStar(side, uv, radius);
				}
			
				skybox.SetSide(axis, pole, side);
			}
		}
	}
}

gfx::ObjectHandle SpawnHolodeckSkybox()
{
	auto skybox = gfx::SkyboxHandle::CreateHandle();
	skybox.Call([] (gfx::Skybox & skybox) {
		DrawHolodeckSkybox(skybox, 512, 16);
	});

	return skybox;
}


gfx::ObjectHandle SpawnStarfieldSkybox()
{
	bool const fast = true;
	auto skybox = gfx::SkyboxHandle::CreateHandle();
	skybox.Call([fast] (gfx::Skybox & object) {
		if (fast)
		{
			DrawStarsFast(object, 512, 20000);
		}
		else
		{
			DrawStarsSlow(object, 256, 100);
		}
	});

	return skybox;
}

gfx::ObjectHandle SpawnBitmapSkybox(std::array<char const *, 6> const & filenames)
{
	auto skybox = gfx::SkyboxHandle::CreateHandle();

	skybox.Call([filenames] (gfx::Skybox & object) {
		gfx::Image image;
	
		auto filename_iterator = std::begin(filenames);
		for (auto axis = 0; axis < 3; ++ axis)
		{
			for (auto pole = 0; pole < 2; ++ filename_iterator, ++ pole)
			{
				image.Load(* filename_iterator);
				object.SetSide(axis, pole, image);
			}
		}

		ASSERT(filename_iterator == std::end(filenames));
	});
	
	return skybox;
}
