//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// light.glsl function which calculates the lighting for the given fragment
lowp vec4 ForegroundLightAll(in highp vec3 position, in lowp vec4 diffuse);

// per-object inputs from the renderer
uniform highp mat4 model_view_matrix;
uniform highp mat4 projection_matrix;
uniform lowp vec4 color;

// per-vertex inputs from renderer
attribute highp vec3 vertex_position;
attribute lowp vec4 vertex_color;

// outputs to disk.frag
varying highp vec3 frag_position;
varying lowp vec4 frag_color;

void main(void)
{
	highp vec4 frag_position4 = model_view_matrix * vec4(vertex_position, 1);
	gl_Position = projection_matrix * frag_position4;
	
	frag_position = frag_position4.xyz;
	frag_color = ForegroundLightAll(frag_position, color);
}
