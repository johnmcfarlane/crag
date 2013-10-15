//
//  light.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
#endif

//#define TEST_NORMALS


// contains information for a single light
struct Light
{
	vec3 position;
	vec3 color;
};


// constants
const int max_lights = 7;


// light information provided by the renderer
uniform Light lights[max_lights];
uniform bool shadows_enabled;

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal, float shadow)
{
	highp vec3 frag_to_light = light.position - frag_position;
	highp float distance = length(frag_to_light);
	
	highp float dp = dot(frag_to_light, frag_normal);
	highp float attenuation = max(shadow * dp / (distance * distance * distance), 0.0);

	lowp vec3 diffuse = light.color * attenuation;
	
	return diffuse;
}

// support function to calculate the light seen on a given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse, float shadow)
{
#if defined(TEST_NORMALS)
	return frag_normal;
#endif

	lowp vec3 illumination = LightFragment(lights[0], frag_position, frag_normal, shadows_enabled ? shadow : 1.);
	
	for (int i = 1; i < max_lights; ++ i)
	{
		illumination += LightFragment(lights[i], frag_position, frag_normal, 1.);
	}

	return diffuse * illumination;
}
