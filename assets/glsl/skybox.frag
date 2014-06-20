//
//  skybox.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform sampler2D texture;

// inputs from skybox.vert
varying COLOR3 fragment_position;
varying lowp vec2 fragment_tex_coord;

void main(void)
{
	COLOR4 texture_color = texture2D(texture, fragment_tex_coord);

	VECTOR3 fragment_direction = normalize(fragment_position);

	gl_FragColor.rgb = BackgroundLightFragment(fragment_direction, texture_color.rgb);
	gl_FragColor.a = 1.;
}
