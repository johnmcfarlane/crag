//
//  poly_v.vert
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

// outputs to poly_v.frag
varying highp vec3 fragment_position;
varying highp vec3 fragment_normal;
varying lowp vec4 fragment_diffuse;
varying lowp vec3 fragment_reflection;
varying lowp vec3 fragment_illumination;

// light.glsl function which calculates the lighting for the given fragment
void ForegroundLightVertex(in highp vec3 position, in highp vec3 normal, out lowp vec3 reflection, out lowp vec3 illumination);

void main(void)
{
	highp vec4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	gl_Position = projection_matrix * position4;
	fragment_position = position4.xyz;

	fragment_normal = (model_view_matrix * vec4(vertex_normal, 0)).xyz;

	fragment_diffuse = color * vertex_color * (1. / 256.);

	ForegroundLightVertex(
		fragment_position, 
		fragment_normal, 
		fragment_reflection, 
		fragment_illumination);
}
