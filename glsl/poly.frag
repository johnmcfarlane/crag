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
void SetFragmentDepth(in vec4 view_position);


// inputs from poly.vert
varying vec3 normal;
varying vec4 position;
varying vec4 color;


void main(void)
{
	gl_FragColor = color * vec4(LightFragment(position.xyz, normalize(normal)), color.a);
	SetFragmentDepth(position);
}
