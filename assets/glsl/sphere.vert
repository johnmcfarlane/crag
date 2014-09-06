//
//  sphere.vert
//  crag
//
//  Created by John McFarlane on 2014-06-15.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform MATRIX4 model_view_matrix;
uniform MATRIX4 projection_matrix;

// per-vertex inputs from renderer
attribute VECTOR3 vertex_position;

// outputs to fragment shader
varying VECTOR4 frag_position;

void main(void)
{
	VECTOR4 vertex_position4 = vec4(vertex_position, 1);
	frag_position = model_view_matrix * vertex_position4;
	gl_Position = projection_matrix * frag_position;
}
