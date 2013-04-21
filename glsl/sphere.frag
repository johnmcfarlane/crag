#version 120

//
//  sphere.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


//#define TEST_INTERSECTION


// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position, in vec3 frag_normal);


// inputs from sphere.vert
varying vec4 quad_position;

// inputs from the renderer
uniform mat4 projection_matrix;
uniform vec4 color;
uniform vec3 center;
uniform float radius;


// given a ray cast from the origin (eye),
// returns point at which it intersects the sphere given by center/radius.
bool GetIntersection(in vec3 ray, out float t)
{
	float a = dot(ray, ray);
	float half_b = dot(ray, center);
	float c = dot(center, center) - (radius * radius);
	
	float root = (half_b * half_b) - a * c;
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
	vec2 clipZW = view_position.z * projection_matrix[2].zw + projection_matrix[3].zw;
	
	gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
}


#if defined(TEST_INTERSECTION)

void main(void)
{
	float t;
	if (! GetIntersection(quad_position.xyz, t))
	{
		gl_FragColor = vec4(1, 0, 0, 1);
	}
	else
	{
		float c = length(center);
		float prop = (c - length(quad_position.xyz * t)) / radius;
		gl_FragColor = vec4(0., prop, prop, 1.);
	}
}

#else

void main(void)
{
	float t;
	if (! GetIntersection(quad_position.xyz, t))
	{
		discard;
	}

	vec4 frag_position = quad_position * t;
	SetFragmentDepth(frag_position);

	vec3 frag_normal = normalize(frag_position.xyz - center);

	vec3 light = LightFragment(frag_position.xyz, frag_normal);
	gl_FragColor = color * vec4(light, 1.);
}

#endif
