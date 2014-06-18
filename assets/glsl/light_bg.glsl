//
//  light_bg.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

////////////////////////////////////////////////////////////////////////////////
// background surfaces (skybox)

// return consolidated light reflected and illuminated by given lights from a given direction
void BackgroundLight(in TypeLights search_lights, in highp vec3 ray_direction, inout lowp vec3 illumination)
{
	for (int i = 0, num_search_lights = search_lights.num_lights; i != num_search_lights; ++ i)
	{
		illumination += GetBeamIllumination(search_lights.lights[i], ray_direction, far_positive);
	}
}

// return consolidated light reflected and illuminated by all lights from a given direction
lowp vec3 BackgroundLightFragment(in highp vec3 ray_direction, in lowp vec3 diffuse)
{
	lowp vec3 color = diffuse;
	BackgroundLight(lights.resolutions[0].types[1], ray_direction, color);
	BackgroundLight(lights.resolutions[1].types[1], ray_direction, color);
	return color;
}
