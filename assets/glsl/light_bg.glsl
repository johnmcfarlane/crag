//
//  light_bg.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(ENABLE_LIGHTING) && defined(ENABLE_BEAM_LIGHTING)

////////////////////////////////////////////////////////////////////////////////
// background surfaces (skybox)

// return consolidated light reflected and illuminated by given lights from a given direction
void BackgroundLight(Light light, VECTOR3 ray_direction, inout COLOR3 illumination)
{
	if (light.type == 1)
	{
		illumination += GetBeamIllumination(light, ray_direction, far_positive);
	}
}

// return consolidated light reflected and illuminated by all lights from a given direction
COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	COLOR3 color = diffuse;
	
#if defined(MAX_FRAGMENT_LIGHTS)
	// no point considering vertex lighting; it should not apply to backgrounds
	for (int i = 0; i != MAX_FRAGMENT_LIGHTS; ++ i)
	{
		BackgroundLight(fragment_lights[i], ray_direction, color);
	}
#endif
	
	return color;
}

#else

COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING
