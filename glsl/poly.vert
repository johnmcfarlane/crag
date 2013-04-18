#version 120

//
//  poly.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform bool fragment_lighting = true;
uniform bool flat_shade = false;

// per-vertex inputs from renderer
attribute vec3 vertex_position;
attribute vec3 vertex_normal;

// outputs to poly.frag
varying vec3 normal;
varying vec3 position;
varying vec4 color;

// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);

void main(void)
{
	position = (gl_ModelViewMatrix * vec4(vertex_position, 1)).xyz;
	normal = normalize(gl_NormalMatrix * vertex_normal);

	if (fragment_lighting || flat_shade)
	{
		color = gl_Color;
	}
	else
	{
		color = gl_Color * vec4(LightFragment(position.xyz, normalize(normal)), gl_Color.a);
	}

	gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex_position, 1);
}
