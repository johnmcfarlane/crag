//
//  flag_enabled.glsl
//  crag
//
//  Created by John McFarlane on 2013-10-30.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2013 John McFarlane. All rights reserved.
//

// returns the same normal for an entire polygon
lowp vec3 FlatNormal(in highp vec3 frag_position, in highp vec3 frag_normal)
{
	lowp vec3 dx = dFdx(fragment_position);
	lowp vec3 dy = dFdy(fragment_position);
	vec3 normal = normalize(cross(dy, dx));

	lowp float d = dot(normal, fragment_normal);
	normal *= d / abs(d);
	
	return normal;
}
