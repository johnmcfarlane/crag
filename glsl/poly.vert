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
uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform vec4 color;
uniform bool fragment_lighting = true;
uniform bool flat_shade = false;

// per-vertex inputs from renderer
attribute vec3 vertex_position;
attribute vec3 vertex_normal;

// outputs to poly.frag
varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec4 fragment_color;

// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);

void main(void)
{
	vec4 position4 = model_view_matrix * vec4(vertex_position, 1);
	fragment_position = position4.xyz;

	fragment_normal = normalize(model_view_matrix * vec4(vertex_normal, 0)).xyz;

	if (fragment_lighting || flat_shade)
	{
		fragment_color = color;
	}
	else
	{
		fragment_color = color * vec4(LightFragment(fragment_position.xyz, normalize(fragment_normal)), color.a);
	}

	gl_Position = projection_matrix * position4;
}
