//
//  SpawnEntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SpawnSkybox.h"

#include "gfx/Engine.h"
#include "gfx/Image.h"
#include "gfx/Texture2d.h"
#include "gfx/TextureCubeMap.h"
#include "gfx/object/Skybox.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "core/ResourceManager.h"

using namespace gfx;

namespace
{
	CONFIG_DEFINE(skybox_starfield_fast, true);
	
	Image CreateHolodeckSkyboxImage(int box_edge_size, int num_bars)
	{
		Image image;
		image.Create(geom::Vector2i(box_edge_size, box_edge_size));
		image.Clear(Color4b::Black());
		auto square_size = box_edge_size / num_bars;
		auto color = Color4f::Cyan();

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
		
		return image;
	}

	Image CreateStarsSlow(int axis, int pole, int box_edge_size, int num_stars)
	{
		int x_axis = TriMod(axis + 1);
		int y_axis = TriMod(axis + 2);
		int z_axis = axis;

		Image image;
		image.Create(geom::Vector2i(box_edge_size, box_edge_size));
		image.Clear(Color4b::Black());

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
				auto axes = star.center.data();
				//float & star_x = axes[x_axis];
				//float & star_y = axes[y_axis];
				float const & star_z = axes[z_axis];

				geom::Vector3f line_direction;
				auto line_axes = line_direction.data();
				float & line_x = line_axes[x_axis];
				float & line_y = line_axes[y_axis];
				float & line_z = line_axes[z_axis];

				for (int i = num_stars; i; -- i)
				{
					star.center.x = random.GetFloatInclusive<float>() - .5f;
					star.center.y = random.GetFloatInclusive<float>() - .5f;
					star.center.z = random.GetFloatInclusive<float>() - .5f;
					star.radius = Squared(random.GetFloatInclusive<float>());

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
					float a_sq = MagnitudeSq(star.center) - Squared(dp);

					intensity += (double)star.radius / (double)a_sq;
				}

				float comp = static_cast<float>(std::min(1., .0002 * intensity));
				Color4f c (comp);
				image.SetPixel(pos, c);
			}
		}

		return image;
	}

	void DrawStar(Image & side, geom::Vector2f const & uv, float r)
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
		Color4f c (comp);
		side.SetPixel(pos, c);
	}

	Image CreateStarsFast(int axis, int pole, int box_edge_size, int num_stars)
	{
		Image image;
		image.Create(geom::Vector2i(box_edge_size, box_edge_size));
		image.Clear(Color4b::Black());
	
		Random random (1);
		for (int i = num_stars; i; -- i)
		{
			geom::Sphere<float, 3> star;
			//random.GetGaussians(star.center.x, star.center.y);
			//random.GetGaussians(star.center.z, star.radius);
			//star.radius = Abs(star.radius);
			star.center.x = random.GetFloatInclusive<float>() - .5f;
			star.center.y = random.GetFloatInclusive<float>() - .5f;
			star.center.z = random.GetFloatInclusive<float>() - .5f;
			star.radius = Squared(random.GetFloatInclusive<float>());
		
			auto axes = star.center.data();
			float w = axes[axis];
			if ((w > 0) != (pole != 0) || w == 0)
			{
				continue;
			}
		
			float w_co = .5f / w;
		
			geom::Vector2f uv(axes[TriMod(axis + 1)] * w_co, axes[TriMod(axis + 2)] * w_co);
			float radius = star.radius * Abs(w_co);
			DrawStar(image, uv, radius);
		}
	
		return image;
	}
}

ObjectHandle SpawnHolodeckSkybox()
{
	Daemon::Call([] (Engine & engine) {
		auto & resource_manager = engine.GetResourceManager();
		
		resource_manager.Register<TextureCubeMap>("HolodeckSkyboxTextures", [] () {
			// pretty inefficient, but the holodeck's a pretty special case
			return TextureCubeMap(CubeMap<Image> ({{
				{{ CreateHolodeckSkyboxImage(512, 16), CreateHolodeckSkyboxImage(512, 16) }},
				{{ CreateHolodeckSkyboxImage(512, 16), CreateHolodeckSkyboxImage(512, 16) }},
				{{ CreateHolodeckSkyboxImage(512, 16), CreateHolodeckSkyboxImage(512, 16) }} }}));
		});
	});
	
	return SkyboxHandle::Create("HolodeckSkyboxTextures");
}


ObjectHandle SpawnStarfieldSkybox()
{
	Daemon::Call([] (Engine & engine) {
		auto & resource_manager = engine.GetResourceManager();
		
		resource_manager.Register<TextureCubeMap>("StarfieldSkyboxTextures", [] () {
			CubeMap<Image> images;
			ForEachSide([& images] (int axis, int pole) {
				if (skybox_starfield_fast)
				{
					images[axis][pole] = CreateStarsFast(axis, pole, 512, 20000);
				}
				else
				{
					images[axis][pole] = CreateStarsSlow(axis, pole, 512, 100);
				}
			});
			return TextureCubeMap(images);
		});
	});
	
	return SkyboxHandle::Create("StarfieldSkyboxTextures");
}

ObjectHandle SpawnBitmapSkybox(std::array<char const *, 6> const & filenames)
{
	Daemon::Call([filenames] (Engine & engine) {
		auto & resource_manager = engine.GetResourceManager();
		
		resource_manager.Register<TextureCubeMap>("BitmapSkyboxTextures", [filenames] () {
			CubeMap<Image> images;
			
			auto filename_iterator = std::begin(filenames);
			ForEachSide([&] (int axis, int pole) {
				images[axis][pole].Load(* filename_iterator);
				
				++ filename_iterator;
			});
			ASSERT(filename_iterator == std::end(filenames));
		
			return TextureCubeMap(images);
		});
	});
	
	return SkyboxHandle::Create("BitmapSkyboxTextures");
}
