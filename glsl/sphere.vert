#version 120

//
//  sphere.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// outputs to sphere.frag
varying vec3 quad_position;
varying vec4 color;


void main(void)
{
	quad_position = vec3(gl_ModelViewMatrix * gl_Vertex);
	color = gl_Color;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
