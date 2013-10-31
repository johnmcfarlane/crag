//
//  flag_disabled.glsl
//  crag
//
//  Created by John McFarlane on 2013-10-30.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2013 John McFarlane. All rights reserved.
//

// returns the same normal for an entire polygon
// (version for hardware that does not support GL_OES_standard_derivatives)
lowp vec3 FlatNormal(in highp vec3 frag_position, in highp vec3 frag_normal)
{
	return frag_normal;
}
