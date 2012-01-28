#version 120

//
//  poly.vert
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


//#define PER_FRAGMENT_LIGHTING


#if ! defined(PER_FRAGMENT_LIGHTING)
// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);
#endif


// outputs to poly.frag
varying vec3 normal;
varying vec4 position;
varying vec4 color;


void main(void)
{
	position = gl_ModelViewMatrix * gl_Vertex;
	normal = normalize(gl_NormalMatrix * gl_Normal);
#if defined(PER_FRAGMENT_LIGHTING)
	color = gl_Color;
#else
	color = gl_Color * vec4(LightFragment(position.xyz, normalize(normal)), gl_Color.a);
#endif
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
