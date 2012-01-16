#version 120

//
//  disk.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position);


// inputs from sphere.vert
varying vec4 quad_position;

// inputs from the renderer
uniform vec4 color;
uniform vec3 center;
uniform float radius;


void main(void)
{
	if (distance(quad_position.xyz, center) > radius)
	{
		discard;
	}
	
	// TODO: Put this in a vert shader.
	vec3 light = LightFragment(center);
	gl_FragColor = color * vec4(light, 1.);
}
