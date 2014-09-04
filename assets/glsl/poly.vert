//
//  poly_v.vert
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
attribute VECTOR3 vertex_normal;
attribute COLOR4 vertex_color;

// outputs to poly_v.frag
varying VECTOR3 fragment_position;
varying VECTOR3 fragment_normal;
varying COLOR4 fragment_diffuse;
varying COLOR3 fragment_reflection;
varying COLOR3 fragment_illumination;

void main(void)
{
	VECTOR4 position4 = model_view_matrix * vec4(vertex_position, 1.);
	gl_Position = projection_matrix * position4;
	fragment_position = position4.xyz;

	fragment_normal = (model_view_matrix * vec4(vertex_normal, 0.)).xyz;

	fragment_diffuse = color * vertex_color * (1. / 256.);

	LightResults result = ForegroundLightVertex(fragment_position, fragment_normal);

	fragment_reflection = result.reflection;
	fragment_illumination = result.illumination;
}
