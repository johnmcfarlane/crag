#version 120

//
//  poly.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// outputs to poly.frag
varying vec3 normal;
varying vec3 position;
varying vec4 color;


void main(void)
{
	position = vec3(gl_ModelViewMatrix * gl_Vertex);
	normal = normalize(gl_NormalMatrix * gl_Normal);
	color = gl_Color;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
