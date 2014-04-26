//
//  poly_v.frag
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

// inputs from poly_v.vert
varying lowp vec4 fragment_color;

void main(void)
{
	gl_FragColor = fragment_color;
}
