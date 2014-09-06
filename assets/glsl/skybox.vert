//
//  skybox.vert
//  crag
//
//  Created by John McFarlane on 2013-04-14.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform MATRIX4 model_view_matrix;
uniform MATRIX4 projection_matrix;

// per-vertex inputs from renderer
attribute VECTOR3 vertex_position;

// outputs to skybox.frag
varying VECTOR3 fragment_position;
varying UV_COORD3 fragment_tex_coord;

void main(void)
{
	VECTOR4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	VECTOR4 vertex = projection_matrix * position4;

	fragment_position = position4.xyz;
	fragment_tex_coord = vertex_position;

	gl_Position = vec4(vertex.xyw, vertex.w);
}
