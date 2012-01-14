#version 120

//
//  light.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// contains information for a single light
struct Light
{
	vec3 position;
	vec3 color;
};


// constants
const bool test_normals = false;
const int max_lights = 7;


// light information provided by the renderer
uniform Light lights[max_lights];


// support function to calculate the light seen on a given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal)
{
	if (test_normals)
	{
		return frag_normal;
	}

	vec3 color = vec3(0,0,0);
	
	for (int i = 0; i < max_lights; ++ i)
	{
		vec3 frag_to_light = lights[i].position - frag_position;
		float distance = length(frag_to_light);
		
		float dp = dot(frag_to_light, frag_normal);
		float attenuation = max(dp / (distance * distance * distance), 0.0);

		vec3 diffuse = lights[i].color * attenuation;
		
		color += diffuse;
	}

	return color;
}

void SetFragmentDepth(in vec4 view_position)
{
	vec2 clipZW = view_position.z * gl_ProjectionMatrix[2].zw + gl_ProjectionMatrix[3].zw;

	gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
}
