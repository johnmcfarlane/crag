//
//  sprite.vert
//  crag
//
//  Created by John McFarlane on 2013-04-14.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform vec2 position_scale;
uniform vec2 position_offset;

// inputs from renderer
attribute vec2 vertex_position;
attribute vec2 vertex_tex_coord;

// outputs to skybox.frag
varying vec2 fragment_tex_coord;

void main(void)
{
	vec2 position = vertex_position * position_scale + position_offset;
	gl_Position = vec4(position.xy, 0, .5);
	fragment_tex_coord = vertex_tex_coord;
}
