//
//  fog.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GLES2
precision highp float;
#endif

//#define TEST_INTERSECTION


// light.frag function which calculates the lighting for the given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse, float shadow);


// inputs from sphere.vert
varying highp vec4 quad_position;

// inputs from the renderer
uniform highp vec4 color;
uniform highp vec3 center;
uniform highp float radius;

// a density of 1. blocks 50% of light across a unit of distance
uniform highp float density;

#if 0
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
#endif

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

#elif defined(OUT_OF_ORDER)

void main(void)
{
	highp float t1, t2;
#if 0
	if (! GetIntersection(quad_position.xyz, t1, t2) || t2 < 0.)
	{
		discard;
	}
#endif
	t1 = max(t1, 0.);	// TODO: near plane?
	
	highp vec4 frag_position = quad_position * t1;
#if 0
	SetFragmentDepth(frag_position);
#endif
	highp vec3 frag_normal = normalize(frag_position.xyz - center);
	
	highp vec4 frag_color = LightFragment(frag_position.xyz, frag_normal, color, 1.);
	
	if (frag_color.a < 0.999)
	{
		// the distance our ray passes through the sphere
		highp float collision_range = length(quad_position.xyz) * (t2 - t1);
		highp float absorption = max(1. - pow(.5, density * collision_range), 0.);
		highp float absorption_alpha = absorption * (1. - frag_color.a);
		
		frag_color += vec4(color.rgb, 1.) * absorption_alpha;
		highp vec4 far_frag_position = quad_position * t2;
#if 0
		SetFragmentDepth(far_frag_position);
#endif
		highp vec3 far_frag_normal = normalize(center - far_frag_position.xyz);
		
		highp vec3 light = LightFragment(far_frag_normal.xyz, far_frag_normal);
		highp float far_alpha = (1. - frag_color.a) * color.a;
		frag_color.a += far_alpha;
		frag_color.rgb += (color.rgb * light.rgb) * far_alpha;
	}
	
	gl_FragColor = clamp(frag_color, 0., 1.);
}

#else
void main(void)
{
}
#endif
