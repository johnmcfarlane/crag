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

// inputs from the renderer
uniform bool fragment_lighting = true;
uniform bool flat_shade = false;

// inputs from poly.vert
varying vec3 normal;
varying vec3 position;
varying vec4 color;

void main(void)
{
	if (flat_shade)
	{
		vec3 dx = dFdx(position);
		vec3 dy = dFdy(position);
		vec3 n = normalize(cross(dy, dx));

		// TODO: argh!!
		if (dot(n, normal) < 0)
			n = - n;

		gl_FragColor = color * vec4(LightFragment(position.xyz, n), color.a);
	}
	else if (fragment_lighting)
	{
		gl_FragColor = color * vec4(LightFragment(position.xyz, normalize(normal)), color.a);
	}
	else
	{
		gl_FragColor = color;
	}
}
