#version 120

//
//  disk.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//


//#define TEST_INTERSECTION


// light.frag function which calculates the lighting for the given fragment
vec3 LightFragment(in vec3 frag_position);


// inputs from sphere.vert
varying vec4 quad_position;

// inputs from the renderer
uniform vec4 color;
uniform vec3 center;
uniform float radius;


// given a ray cast from the origin (eye),
// returns point at which it intersects the sphere given by center/radius.
bool GetIntersection(in vec3 ray)
{
	float a = dot(ray, ray);
	float half_b = dot(ray, center);
	float c = dot(center, center) - (radius * radius);
	
	float root = (half_b * half_b) - a * c;
	return root >= 0.;
}


void SetFragmentDepth(in vec4 view_position)
{
	vec2 clipZW = view_position.z * gl_ProjectionMatrix[2].zw + gl_ProjectionMatrix[3].zw;
	
	gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
}


#if defined(TEST_INTERSECTION)

void main(void)
{
	float t1, t2;
	if (! GetIntersection(quad_position.xyz, t1, t2))
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
		float prop1 = (c - length(quad_position.xyz * t1)) / radius;
		float prop2 = (length(quad_position.xyz * t2) - c) / radius;
		gl_FragColor = vec4(0., prop1, prop2, 1.);
	}
}

#else

void main(void)
{
	if (! GetIntersection(quad_position.xyz))
	{
		discard;
	}
	
	// TODO: Put this in a vert shader.
	vec3 light = LightFragment(center);
	gl_FragColor = color * vec4(light, 1.);
}

#endif
