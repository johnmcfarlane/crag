//
//  sphere.frag
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
highp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal);


// inputs from sphere.vert
varying highp vec4 quad_position;
	
// inputs from the renderer
uniform highp mat4 projection_matrix;
uniform highp vec4 color;
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
	highp vec2 clipZW = view_position.z * projection_matrix[2].zw + projection_matrix[3].zw;
	
	//gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
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
	highp float t;
	if (! GetIntersection(quad_position.xyz, t))
	{
		discard;
	}

	highp vec4 frag_position = quad_position * t;
	SetFragmentDepth(frag_position);

	highp vec3 frag_normal = normalize(frag_position.xyz - center);

	highp vec3 light = LightFragment(frag_position.xyz, frag_normal);
	gl_FragColor = color * vec4(light, 1.);
}

#endif