//
//  disk.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
#endif

// inputs from the renderer
uniform highp vec3 center;
uniform highp float radius;	// TODO: use inverse radius instead

// inputs from disk.vert
varying highp vec3 frag_position;
varying highp vec4 frag_color;

void main(void)
{
	highp float d = distance(frag_position, center) / radius;
	if (d > 1.)
	{
		discard;
	}

	d *= d * d;
	d = 1. - d;
	gl_FragColor = vec4(frag_color.xyz, frag_color.a * d);
}
