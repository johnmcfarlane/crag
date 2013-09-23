//
//  sprite.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
#endif

// inputs from the renderer
uniform sampler2D texture;

// inputs from skybox.vert
varying lowp vec2 fragment_tex_coord;

void main(void)
{
	gl_FragColor = texture2D(texture, fragment_tex_coord);
}
