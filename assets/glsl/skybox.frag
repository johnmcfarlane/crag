//
//  skybox.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
#endif

// per-object inputs from the renderer
uniform sampler2D texture;

// inputs from skybox.vert
varying lowp vec3 fragment_position;
varying lowp vec2 fragment_tex_coord;

// light.glsl function which calculates the lighting for the given fragment
lowp vec3 BackgroundLightFragment(in highp vec3 ray_direction, in lowp vec3 diffuse);

void main(void)
{
	vec4 texture_color = texture2D(texture, fragment_tex_coord);

	vec3 fragment_direction = normalize(fragment_position);

	gl_FragColor.rgb = BackgroundLightFragment(fragment_direction, texture_color.rgb);
	gl_FragColor.a = 1.;
}
