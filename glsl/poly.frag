#version 120

//
//  poly.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from the renderer
uniform bool fragment_lighting = true;
uniform bool flat_shade = false;

// inputs from poly.vert
varying vec3 normal;
varying vec3 position;
varying vec4 color;

// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);

void main(void)
{
	gl_FragColor = color;
	
	if (flat_shade)
	{
		vec3 dx = dFdx(position);
		vec3 dy = dFdy(position);
		vec3 n = normalize(cross(dy, dx));

		// TODO: argh!!
		float d = dot(n, normal);
		n *= d / abs(d);

		gl_FragColor *= vec4(LightFragment(position.xyz, n), color.a);
	}
	else if (fragment_lighting)
	{
		gl_FragColor *= vec4(LightFragment(position.xyz, normalize(normal)), color.a);
	}
}
