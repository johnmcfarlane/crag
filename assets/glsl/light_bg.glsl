//
//  light_bg.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(ENABLE_LIGHTING)

////////////////////////////////////////////////////////////////////////////////
// background surfaces (skybox)

// return consolidated light reflected and illuminated by given lights from a given direction
void BackgroundLight(in TypeLights search_lights, in VECTOR3 ray_direction, inout COLOR3 illumination)
{
	for (int i = search_lights.num_lights; i -- > 0;)
	{
		illumination += GetBeamIllumination(search_lights.lights[i], ray_direction, far_positive);
	}
}

// return consolidated light reflected and illuminated by all lights from a given direction
COLOR3 BackgroundLightFragment(in VECTOR3 ray_direction, in COLOR3 diffuse)
{
	COLOR3 color = diffuse;
	BackgroundLight(lights.resolutions[0].types[1], ray_direction, color);
	BackgroundLight(lights.resolutions[1].types[1], ray_direction, color);
	return color;
}

#else

COLOR3 BackgroundLightFragment(in VECTOR3 ray_direction, in COLOR3 diffuse)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING
