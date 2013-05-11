//
//  poly.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#extension GL_OES_standard_derivatives : enable

// inputs from the renderer
uniform bool fragment_lighting;
uniform bool flat_shade;

// inputs from poly.vert
varying vec3 fragment_normal;
varying vec3 fragment_position;
varying vec4 fragment_color;

// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);

void main(void)
{
	gl_FragColor = fragment_color;
	
	if (flat_shade)
	{
		vec3 dx = dFdx(fragment_position);
		vec3 dy = dFdy(fragment_position);
		vec3 n = normalize(cross(dy, dx));

		// TODO: argh!!
		float d = dot(n, fragment_normal);
		n *= d / abs(d);

		gl_FragColor *= vec4(LightFragment(fragment_position.xyz, n), fragment_color.a);
	}
	else if (fragment_lighting)
	{
		gl_FragColor *= vec4(LightFragment(fragment_position.xyz, normalize(fragment_normal)), fragment_color.a);
	}
}
