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
	vec3 direction;	// for beams
	vec4 color;
};


// constants
const int max_lights = 8;


// light information provided by the renderer
uniform vec4 ambient;
uniform int num_point_lights;
uniform int num_beam_lights;
uniform Light lights[max_lights];

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment_Point(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal)
{
	highp vec3 frag_to_light = light.position - frag_position;
	highp float distance = length(frag_to_light);
	
	highp float dp = dot(frag_to_light, frag_normal);
	highp float attenuation = max(dp / (distance * distance * distance), 0.0);

	lowp vec3 color = light.color.rgb * attenuation;
	
	return color;
}

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment_Beam(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal)
{
	highp vec3 frag_to_light_source = light.position - frag_position;
	highp float r = length(frag_to_light_source - light.direction * dot(frag_to_light_source, light.direction));
	highp float i = exp((-5000. * r * r));
	
	return light.color.rgb * i;
}

// support function to calculate the light seen on a given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse)
{
#if defined(TEST_NORMALS)
	return frag_normal;
#endif

	lowp vec3 illumination = ambient.rgb;
	
	for (int i = 0; i < num_point_lights; ++ i)
	{
		illumination += LightFragment_Point(lights[i], frag_position, frag_normal);
	}
	
	int num_lights = num_point_lights + num_beam_lights;
	for (int i = num_point_lights; i < num_lights; ++ i)
	{
		illumination += LightFragment_Beam(lights[i], frag_position, frag_normal);
	}
	
	return diffuse * illumination;
}
