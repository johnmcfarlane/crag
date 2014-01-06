//
//  light.glsl
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

//#define TEST_NORMALS


// contains information for a single light
struct Light
{
	vec3 position;
	vec3 color;
};


// constants
const int max_lights = 7;	// TODO: still quite hacky


// light information provided by the renderer
uniform Light lights[max_lights];
uniform int num_lights;

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal)
{
	highp vec3 frag_to_light = light.position - frag_position;
	highp float distance = length(frag_to_light);
	
	highp float dp = dot(frag_to_light, frag_normal);
	highp float attenuation = max(dp / (distance * distance * distance), 0.0);

	lowp vec3 color = light.color * attenuation;
	
	return color;
}

// support function to calculate the light seen on a given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse)
{
#if defined(TEST_NORMALS)
	return frag_normal;
#endif

	lowp vec3 illumination = vec3(0);
	
	for (int i = 0; i < num_lights; ++ i)
	{
		illumination += LightFragment(lights[i], frag_position, frag_normal);
	}
	
	return diffuse * illumination;
}
