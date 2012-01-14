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
varying vec3 quad_position;
varying vec4 color;

// inputs from the renderer
uniform vec3 center;
uniform float radius;


// given a ray cast from the origin (eye),
// returns point at which it intersects the sphere given by center/radius.
bool GetIntersection(in vec3 ray, out float t1, out float t2)
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
	t1 = (half_b - root) / a;
	t2 = (half_b + root) / a;
	
	return true;
}


#if defined(TEST_INTERSECTION)

void main(void)
{
	float t1, t2;
	if (! GetIntersection(quad_position, t1, t2))
	{
		gl_FragColor = vec4(1, 0, 0, 1);
	}
	else if (t2 < 0)
	{
		gl_FragColor = vec4(0, 0, 0, 1);
	}
	else
	{
		float c = length(center);
		float prop1 = (c - length(quad_position * t1)) / radius;
		float prop2 = (length(quad_position * t2) - c) / radius;
		gl_FragColor = vec4(0., prop1, prop2, 1.);
	}
}

#else

void main(void)
{
	float t1, t2;
	if (! GetIntersection(quad_position, t1, t2) || t2 < 0)
	{
		discard;
	}

	vec3 frag_position = quad_position * t1;
	vec3 frag_normal = normalize(frag_position - center);
	
	vec3 light = LightFragment(frag_position, frag_normal);

	if (color.a == 1.)
	{
		gl_FragColor = color * vec4(light, 1.);
	}
	else
	{
		// the distance our ray passes through the sphere
		float depth = length(quad_position) * (t2 - t1);
		gl_FragColor = vec4(color.xyz * light.xyz, 1. - pow(1. - color.a, depth));
	}
	
	gl_FragDepth = gl_FragCoord.z;
}

#endif
