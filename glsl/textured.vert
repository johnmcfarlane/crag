#version 120

//
//  textured.vert
//  crag
//
//  Created by John McFarlane on 2013-04-14.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from renderer
attribute vec3 vertex_position;
attribute vec2 vertex_tex_coord;

// outputs to skybox.frag
varying vec2 fragment_tex_coord;

void main(void)
{
	vec4 vertex = gl_ModelViewProjectionMatrix * vec4(vertex_position, 1.0);
	gl_Position = vec4(vertex.xyw, vertex.w);
	fragment_tex_coord = vertex_tex_coord;
}
