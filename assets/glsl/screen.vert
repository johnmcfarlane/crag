//
//  screen.vert
//  crag
//
//  Created by John McFarlane on 2014-01-06.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

// per-vertex inputs from renderer
attribute vec3 vertex_position;

void main(void)
{
	gl_Position = vec4(vertex_position.xy, 0., 1.);
}
