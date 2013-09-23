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


// support function to calculate the light seen on a given fragment
highp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal)
{
#if defined(TEST_NORMALS)
	return frag_normal;
#endif

	highp vec3 color = vec3(0,0,0);
	
	for (int i = 0; i < max_lights; ++ i)
	{
		highp vec3 frag_to_light = lights[i].position - frag_position;
		highp float distance = length(frag_to_light);
		
		highp float dp = dot(frag_to_light, frag_normal);
		highp float attenuation = max(dp / (distance * distance * distance), 0.0);

		highp vec3 diffuse = lights[i].color * attenuation;
		
		color += diffuse;
	}

	return color;
}
