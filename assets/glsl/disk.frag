//
//  disk.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from the renderer
uniform VECTOR3 center;
uniform SCALAR radius;	// TODO: use inverse radius instead

// inputs from disk.vert
varying VECTOR3 frag_position;
varying VECTOR4 frag_color;

void main(void)
{
	SCALAR d = distance(frag_position, center) / radius;
	if (d > 1.)
	{
		discard;
	}

	d *= d * d;
	d = 1. - d;
	gl_FragColor = vec4(frag_color.xyz, frag_color.a * d);
}
