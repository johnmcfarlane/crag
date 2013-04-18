#version 120

//
//  disk.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-vertex inputs from renderer
attribute vec3 vertex_position;

// outputs to poly.frag
varying vec4 quad_position;

void main(void)
{
	vec4 vertex_position4 = vec4(vertex_position, 1.0f);
	quad_position = gl_ModelViewMatrix * vertex_position4;
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position4;
}
