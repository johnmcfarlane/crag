//
//  poly.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#extension GL_OES_standard_derivatives : enable

#ifdef GL_ES
precision highp float;
precision highp int;
#endif


// inputs from the renderer
uniform bool fragment_lighting;
uniform bool flat_shade;

// inputs from poly.vert
varying lowp vec3 fragment_normal;
varying lowp vec3 fragment_position;
varying lowp vec4 fragment_color;

// light.glsl function which calculates the lighting for the given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse);
lowp vec3 FlatNormal(in highp vec3 frag_position, in highp vec3 frag_normal);

void main(void)
{
	lowp vec3 normal;
	if (flat_shade)
	{
		normal = FlatNormal(fragment_position, fragment_normal);

		gl_FragColor = vec4(LightFragment(fragment_position.xyz, normal, fragment_color.rgb), fragment_color.a);
	}
	else if (fragment_lighting)
	{
		normal = normalize(fragment_normal);

		gl_FragColor = vec4(LightFragment(fragment_position.xyz, normal, fragment_color.rgb), fragment_color.a);
	}
	else
	{
		gl_FragColor = fragment_color;
	}
}
