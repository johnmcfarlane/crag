//
//  skybox.vert
//  crag
//
//  Created by John McFarlane on 2013-04-14.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

// per-vertex inputs from renderer
attribute vec3 vertex_position;
attribute vec2 vertex_tex_coord;

// outputs to skybox.frag
varying vec3 fragment_position;
varying vec2 fragment_tex_coord;

void main(void)
{
	vec4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	vec4 vertex = projection_matrix * position4;

	fragment_position = position4.xyz;
	fragment_tex_coord = vertex_tex_coord;

	gl_Position = vec4(vertex.xyw, vertex.w);
}
