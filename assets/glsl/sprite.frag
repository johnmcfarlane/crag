//
//  sprite.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from the renderer
uniform sampler2D texture;

// inputs from skybox.vert
varying UV_COORD2 fragment_tex_coord;

void main(void)
{
	gl_FragColor = texture2D(texture, fragment_tex_coord);
}
