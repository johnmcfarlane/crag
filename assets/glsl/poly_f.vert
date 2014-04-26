//
//  poly_f.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp int;
#endif

// per-object inputs from the renderer
uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform vec4 color;

// per-vertex inputs from renderer
attribute vec3 vertex_position;
attribute vec3 vertex_normal;
attribute vec4 vertex_color;

// outputs to poly_f.frag
varying vec3 fragment_position;
varying vec3 fragment_normal;
varying vec4 fragment_color;

// light.glsl function which calculates the lighting for the given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse);

void main(void)
{
	highp vec4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	gl_Position = projection_matrix * position4;

	fragment_position = position4.xyz;
	fragment_normal = (model_view_matrix * vec4(vertex_normal, 0)).xyz;
	fragment_color = color * vertex_color * (1. / 256.);
}
