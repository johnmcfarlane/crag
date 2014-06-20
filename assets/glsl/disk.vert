//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform MATRIX4 model_view_matrix;
uniform MATRIX4 projection_matrix;
uniform COLOR4 color;

// per-vertex inputs from renderer
attribute VECTOR3 vertex_position;
attribute COLOR4 vertex_color;

// outputs to disk.frag
varying VECTOR3 frag_position;
varying COLOR4 frag_color;

void main(void)
{
	VECTOR4 frag_position4 = model_view_matrix * VECTOR4(vertex_position, 1);
	gl_Position = projection_matrix * frag_position4;
	
	frag_position = frag_position4.xyz;
	frag_color = ForegroundLightAll(frag_position, color);
}
