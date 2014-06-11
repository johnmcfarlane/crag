//
//  sphere.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
#endif


// light.glsl function which calculates the lighting for the given fragment
lowp vec4 ForegroundLightAll(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse);


// inputs from sphere.vert
varying highp vec4 quad_position;
	
// inputs from the renderer
uniform highp mat4 projection_matrix;
uniform lowp vec4 color;
uniform highp vec3 center;
uniform highp float radius;


// given a ray cast from the origin (eye),
// returns point at which it intersects the sphere given by center/radius.
bool GetIntersection(in highp vec3 ray, out highp float t)
{
	highp float a = dot(ray, ray);
	highp float half_b = dot(ray, center);
	highp float c = dot(center, center) - (radius * radius);
	
	highp float root = (half_b * half_b) - a * c;
	if (root < 0.)
	{
		// no intersection
		return false;
	}
	root = sqrt(root);

	t = (half_b - root) / a;
	return true;
}


void SetFragmentDepth(in vec4 view_position)
{
#ifndef GL_ES
	highp vec2 clipZW = view_position.z * projection_matrix[2].zw + projection_matrix[3].zw;
	
	gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
#endif
}

void main(void)
{
	highp float t;
	if (! GetIntersection(quad_position.xyz, t))
	{
		discard;
	}

	highp vec4 frag_position = quad_position * t;
	SetFragmentDepth(frag_position);

	highp vec3 frag_normal = normalize(frag_position.xyz - center);

	gl_FragColor = ForegroundLightAll(frag_position.xyz, frag_normal.xyz, color);
}
