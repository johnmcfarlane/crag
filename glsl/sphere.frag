#version 120

//
//  sphere.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


// turn this on to check the the correct fragments are being discarded / filled in.
const bool test_intersection = false;


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
float GetIntersection(in vec3 ray)
{
	float a = dot(ray, ray);
	float half_b = dot(ray, center);
	float c = dot(center, center) - (radius * radius);
	
	float root = (half_b * half_b) - a * c;
	if (root < 0.)
	{
		if (test_intersection)
		{
			gl_FragColor = vec4(1,0,0,1);
			return 0.;
		}
		else
		{
			discard;
		}
	}
	root = sqrt(root);

	if (test_intersection)
	{
		gl_FragColor = vec4(0,1,0,1);
		return 0.;
	}
	else
	{
		return (half_b - root) / a;
	}
}


void main(void)
{
	float t = GetIntersection(quad_position);

	if (! test_intersection)
	{
		vec3 frag_position = quad_position * t;
		vec3 frag_normal = normalize(frag_position - center);
		
		vec3 light = LightFragment(frag_position, frag_normal);
		gl_FragColor = color * vec4(light, color.a);
	}
}
