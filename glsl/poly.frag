#version 120

//
//  poly.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);


// inputs from poly.vert
varying vec3 normal;
varying vec3 position;
varying vec4 color;


void main(void)
{
	gl_FragColor = color * vec4(LightFragment(position, normalize(normal)), color.a);
}
