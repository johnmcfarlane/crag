//
//  skybox.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// per-object inputs from the renderer
uniform samplerCube cube_texture;

// inputs from skybox.vert
varying VECTOR3 fragment_position;
varying UV_COORD3 fragment_tex_coord;

void main(void)
{
	COLOR3 texture_color = textureCube(cube_texture, fragment_tex_coord).rgb;

	VECTOR3 fragment_direction = normalize(fragment_position);

	gl_FragColor.rgb = BackgroundLightFragment(fragment_direction, texture_color);
	gl_FragColor.a = 1.;
}
