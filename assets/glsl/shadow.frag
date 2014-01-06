//
//  shadow.frag
//  crag
//
//  Created by John McFarlane on 2014-01-06.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#extension GL_OES_standard_derivatives : enable

#ifdef GL_ES
precision highp float;
#endif

// inputs from shadow.vert
varying lowp vec3 fragment_position;

void main(void)
{
	gl_FragColor = vec4(1., 0., 1., .1);
}
