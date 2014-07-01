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
COLOR3 BackgroundLight(int search_lights_begin, int search_lights_end, VECTOR3 ray_direction)
{
	COLOR3 illumination = COLOR3(0.);
	
	for (int count = search_lights_end - search_lights_begin; count > 0; -- count)
	{
		Light light = lights[search_lights_begin ++];

		illumination += GetBeamIllumination(light, ray_direction, far_positive);
	}

	return illumination;
}

// return consolidated light reflected and illuminated by all lights from a given direction
COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	COLOR3 color = diffuse;
	color += BackgroundLight(vertex_point_lights_end, vertex_search_lights_end, ray_direction);
	color += BackgroundLight(fragment_point_lights_end, fragment_search_lights_end, ray_direction);
	return color;
}

#else

COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING
