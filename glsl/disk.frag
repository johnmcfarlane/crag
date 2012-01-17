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
uniform float radius;

// inputs from disk.vert
varying vec4 quad_position;
varying vec4 lit_color;


void main(void)
{
	if (distance(quad_position.xyz, center) > radius)
	{
		discard;
	}
	
	gl_FragColor = lit_color;
}
