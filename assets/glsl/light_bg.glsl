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
COLOR3 BackgroundLight(VECTOR3 ray_direction, bool fragment)
{
	COLOR3 illumination = COLOR3(0.);
	
	for (int i = 0; i != MAX_LIGHTS; ++ i)
	{
		Light light = lights[i];
		
		if (! (light.used && light.search))
		{
			continue;
		}
		
		if (light.fragment != fragment)
		{
			continue;
		}

		illumination += GetBeamIllumination(light, ray_direction, far_positive);
	}

	return illumination;
}

// return consolidated light reflected and illuminated by all lights from a given direction
COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	COLOR3 color = diffuse;
	color += BackgroundLight(ray_direction, false);
	color += BackgroundLight(ray_direction, true);
	return color;
}

#else

COLOR3 BackgroundLightFragment(VECTOR3 ray_direction, COLOR3 diffuse)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING
