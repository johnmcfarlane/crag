#version 120

//
//  fog.frag
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
uniform vec4 color;
uniform vec3 center;
uniform float radius;

// a density of 1. blocks 50% of light across a unit of distance
uniform float density;


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
	t2 = (half_b + root) / a;
	if (t2 < 0)
	{
		return false;
	}
	
	t1 = (half_b - root) / a;
	return true;
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
	float t1, t2;
	if (! GetIntersection(quad_position.xyz, t1, t2) || t2 < 0)
	{
		discard;
	}
	t1 = max(t1, 0.);	// TODO: near plane?
	
	vec4 frag_position = quad_position * t1;
	SetFragmentDepth(frag_position);
	vec3 frag_normal = normalize(frag_position.xyz - center);
	
	vec3 light = LightFragment(frag_position.xyz, frag_normal);
	vec4 frag_color = color * vec4(light, 1.);
	
	if (frag_color.a < 0.999)
	{
		// the distance our ray passes through the sphere
		float collision_range = length(quad_position.xyz) * (t2 - t1);
		float absorption = max(1. - pow(.5, density * collision_range), 0.);
		float absorption_alpha = absorption * (1. - frag_color.a);
		
		frag_color += vec4(color.rgb, 1.) * absorption_alpha;
		vec4 far_frag_position = quad_position * t2;
		SetFragmentDepth(far_frag_position);
		vec3 far_frag_normal = normalize(center - far_frag_position.xyz);
		
		vec3 light = LightFragment(far_frag_normal.xyz, far_frag_normal);
		float far_alpha = (1. - frag_color.a) * color.a;
		frag_color.a += far_alpha;
		frag_color.rgb += (color.rgb * light.rgb) * far_alpha;
	}
	
	gl_FragColor = clamp(frag_color, 0., 1.);
}

#endif
