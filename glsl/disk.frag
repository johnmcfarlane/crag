#version 120

//
//  disk.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// inputs from the renderer
uniform vec3 center;
uniform float radius;	// TODO: use inverse radius instead
uniform vec4 color;

// inputs from disk.vert
varying vec4 quad_position;


void main(void)
{
	float d = distance(quad_position.xyz, center) / radius;
	if (d > 1)
	{
		//discard;
	}

	d *= d * d;
	d = 1. - d;
	gl_FragColor = vec4(color.xyz, color.a * d);
}
