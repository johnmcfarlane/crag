//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform highp mat4 model_view_matrix;
uniform highp mat4 projection_matrix;

// per-vertex inputs from renderer
attribute highp vec3 vertex_position;

// outputs to poly.frag
varying highp vec4 quad_position;

void main(void)
{
	highp vec4 vertex_position4 = vec4(vertex_position, 1);
	quad_position = model_view_matrix * vertex_position4;
	gl_Position = projection_matrix * quad_position;
}