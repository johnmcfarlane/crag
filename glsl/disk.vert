#version 120

//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


////////////////////////////////////////////////////////////////////////////////
// types

// contains information for a single light
struct Light
{
	vec3 position;
	vec3 color;
};


////////////////////////////////////////////////////////////////////////////////
// constants

// constants
const int max_lights = 7;


////////////////////////////////////////////////////////////////////////////////
// inputs

// light information provided by the renderer
uniform Light lights[max_lights];

uniform vec4 color;


////////////////////////////////////////////////////////////////////////////////
// outputs

varying vec4 quad_position;
varying vec4 lit_color;


////////////////////////////////////////////////////////////////////////////////
// functions

// support function to calculate the light seen on a given fragment
vec3 LightFragment(in vec3 frag_position)
{
	vec3 color = vec3(0,0,0);
	
	for (int i = 0; i < max_lights; ++ i)
	{
		vec3 frag_to_light = lights[i].position - frag_position;
		float distance = length(frag_to_light);
		
		float attenuation = clamp(1. / (distance * distance), 0.0, 1.0);
		
		vec3 diffuse = lights[i].color * attenuation;
		
		color += diffuse;
	}
	
	return color;
}


void main(void)
{
	quad_position = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	vec3 light = LightFragment(quad_position.xyz);
	lit_color = color * vec4(light, 1.);
}
