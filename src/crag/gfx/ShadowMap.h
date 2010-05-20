/*
 *  ShadowMap.h
 *  Crag
 *
 *  Created by John on 12/23/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"
#include "glpp/Texture.h"

#if defined(NDEBUG)
#define SHADOW_MAP_TEST 0	// 0 -> Off
#else
#define SHADOW_MAP_TEST 0	// 1 -> Show Depth Buffer; 2 -> Show Color Buffer
#endif


namespace sim
{
	class Entity;
}


namespace gfx
{
	class Light;


	// TODO: No point mapping specific depth texture to each map/entity combo.
	// Just keep a pool instead.
	class ShadowMapKey
	{
	public:
		Light const * light;
		sim::Entity const * entity;

		friend bool operator<(ShadowMapKey const & lhs, ShadowMapKey const & rhs) {
			return lhs.light < rhs.light || (lhs.light == rhs.light && lhs.entity < rhs.entity);
		}
	};
	
	
	class ShadowMap
	{
	public:
		void Begin(int size);
	private:
		void InitTextures(int size);
		void BindTextures();

	public:
		gl::TextureDepth & GetDepthTexture() { return depth_texture; }
		gl::TextureDepth const & GetDepthTexture() const { return depth_texture; }

	#if (SHADOW_MAP_TEST >= 2)
		gl::TextureRgba8 & GetColorTexture() { return color_texture; }
		gl::TextureRgba8 const & GetColorTexture() const { return color_texture; }
	#endif

		sim::Matrix4 light_proj;
		sim::Matrix4 light_view;
	private:
		gl::TextureDepth depth_texture;

	#if (SHADOW_MAP_TEST >= 2)
		gl::TextureRgba8 color_texture;
	#endif
	};
}
