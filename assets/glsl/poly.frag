//
//  poly_f.frag
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

// inputs from poly_f.vert
varying highp vec3 fragment_position;
varying highp vec3 fragment_normal;
varying lowp vec4 fragment_diffuse;
varying lowp vec3 fragment_reflection;
varying lowp vec3 fragment_illumination;

// light.glsl function which calculates the lighting for the given fragment
lowp vec4 ForegroundLightFragment(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse, in lowp vec3 reflection, in lowp vec3 illumination);

void main(void)
{
	vec3 normal = normalize(fragment_normal);

	gl_FragColor = ForegroundLightFragment(
		fragment_position, 
		fragment_normal, 
		fragment_diffuse, 
		fragment_reflection, 
		fragment_illumination);
}
