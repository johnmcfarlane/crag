//
//  poly.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
precision highp int;
#endif


// inputs from the renderer
uniform bool fragment_lighting;

// inputs from poly.vert
varying lowp vec3 fragment_normal;
varying lowp vec3 fragment_position;
varying lowp vec4 fragment_color;

// light.glsl function which calculates the lighting for the given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse);

void main(void)
{
	vec3 diffuse = fragment_color.rgb;
	
	if (fragment_lighting)
	{
		lowp vec3 normal = normalize(fragment_normal);

		gl_FragColor = vec4(LightFragment(fragment_position.xyz, normal, diffuse.rgb), fragment_color.a);
	}
	else
	{
		gl_FragColor.xyz = diffuse;
		gl_FragColor.a = fragment_color.a;
	}
}
