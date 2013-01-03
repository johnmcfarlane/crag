#version 120

//
//  poly.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);

// inputs from the renderer
uniform bool fragment_lighting = true;

// outputs to poly.frag
varying vec3 normal;
varying vec3 position;
varying vec4 color;

void main(void)
{
	position = (gl_ModelViewMatrix * gl_Vertex).xyz;
	normal = normalize(gl_NormalMatrix * gl_Normal);

	if (fragment_lighting)
	{
		color = gl_Color;
	}
	else
	{
		color = gl_Color * vec4(LightFragment(position.xyz, normalize(normal)), gl_Color.a);
	}

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
