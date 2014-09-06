//
//  shadow.vert
//  crag
//
//  Created by John McFarlane on 2014-01-06.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform MATRIX4 model_view_matrix;
uniform MATRIX4 projection_matrix;

// per-vertex inputs from renderer
attribute VECTOR3 vertex_position;

void main(void)
{
	VECTOR4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	gl_Position = projection_matrix * position4;
}
