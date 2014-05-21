//
//  light.glsl
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

#define PI 3.14159265358979323846264338327950288419716939937510
//#define TEST_NORMALS

const float far_positive = 1000000.;
const float far_negative = - far_positive;

// contains information for a single light
struct Light
{
	vec3 position;
	vec3 direction;	// for search lights
	vec4 color;
	vec2 angle;	// sin/cos
};

// used by search beam functions
struct Contact
{
	vec3 position;
	float t;
};


// constants
const int max_lights = 2;


// light information provided by the renderer
uniform vec4 ambient;
uniform int num_point_lights;
uniform int num_search_lights;
uniform Light lights[max_lights];


float Squared(float s)
{
	return s * s;
}

float Calculate_SearchAttenuation(float sin_angle, float light_to_position)
{
	return Squared(sin_angle * light_to_position);
}

lowp vec3 ApplyFog(in Light light, in float fog_distance, in float light_to_position)
{
	// factor based on how far through the beam the ray passes
	float particle_density = 2.5;
	float fog_factor_from_ray_penetration = .5 * Squared(particle_density * fog_distance);

	// factor relates to falloff from particles blocking light as it travels away from light source
	float particle_falloff = .5;
	float fog_factor_from_light_penetration = exp(- Squared(particle_falloff * light_to_position));

	// attenuation relative to distance from light source
	float intensity_of_light_being_traversed = Calculate_SearchAttenuation(light.angle.x, light_to_position);

	float fog_intensity = fog_factor_from_ray_penetration * fog_factor_from_light_penetration / intensity_of_light_being_traversed;

	return light.color.rgb * fog_intensity;
}

void SetContact(inout Contact contact, in float t, in vec3 camera_direction)
{
	contact.position = t * camera_direction;
	contact.t = t;
}

bool GetContactGood(in Contact contact, in Light light)
{
	return dot(contact.position - light.position, light.direction) < 0.;
}

vec3 LightFragment_SearchBeam(in Light light, in highp vec3 camera_direction, float camera_distance)
{
	// cone/line intersection test copied from
	// http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
	float AdD = dot(light.direction, camera_direction);
	float cosSqr = Squared(light.angle.y);
	vec3 E = - light.position;
	float AdE = dot(light.direction, E);
	float DdE = dot(camera_direction, E);
	float EdE = dot(E, E);
	float c2 = AdD * AdD - cosSqr;

	if (c2 == 0.)
	{
		// haven't seend this happen; would like to know about it if it did.
		return vec3(1., 0., 0.);
	}

	float c1 = AdD * AdE - cosSqr * DdE;
	float c0 = AdE * AdE - cosSqr * EdE;

	float discr = c1 * c1 - c0 * c2;
	if (discr <= 0.)
	{
		return vec3(0.);
	}

	// Q(t) = 0 has two distinct real-valued roots.
	float root = sqrt(discr);
	float invC2 = 1. / c2;

	Contact contact1, contact2;
	SetContact(contact1, (- c1 - root) * invC2, camera_direction);
	SetContact(contact2, (- c1 + root) * invC2, camera_direction);
	bool good1 = GetContactGood(contact1, light);
	bool good2 = GetContactGood(contact2, light);

	if (good1)
	{
		if (good2)
		{
			// very good
		}
		else
		{
			float t = (contact1.t < contact2.t) ? far_negative : far_positive;
			SetContact(contact2, t, camera_direction);
		}
	}
	else
	{
		if (good2)
		{
			float t = (contact2.t < contact1.t) ? far_negative : far_positive;
			SetContact(contact1, t, camera_direction);
		}
		else
		{
			return vec3(0.);
		}
	}

	float t1 = min(contact1.t, contact2.t);
	float t2 = max(contact1.t, contact2.t);

	if (t1 > camera_distance || t2 < 0.)
	{
		return vec3(0.);
	}

	if (t1 < 0.)
	{
		t1 = t2;
		t2 = 1000000.;
	}
	if (t2 > camera_distance)
	{
		t2 = camera_distance;
	}

	if (t1 >= t2)
	{
		return vec3(0.);
	}

	float depth = min(t2 - t1, 1000000000.);
	vec3 avg_light_position = (contact1.position + contact2.position) * .5;
	return ApplyFog(light, depth, length(avg_light_position - light.position));
}

vec3 LightFragment_SearchBeam(in Light light, in highp vec3 frag_position)
{
	float camera_distance = length(frag_position);
	vec3 camera_direction = frag_position / camera_distance;

	return LightFragment_SearchBeam(light, camera_direction, camera_distance);
}

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment_Point(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal)
{
	highp vec3 frag_to_light = light.position - frag_position;
	highp float distance = length(frag_to_light);
	
	highp float dp = dot(frag_to_light, frag_normal);
	highp float attenuation = max(dp / (distance * distance * distance), 0.0);

	lowp vec3 color = light.color.rgb * attenuation;
	
	return color;
}

// support function to calculate the light shone on a given fragment by a given light
lowp vec3 LightFragment_Search(in Light light, in highp vec3 frag_position, in highp vec3 frag_normal)
{
	vec3 frag_to_light_source = light.position - frag_position;
	float d = length(frag_to_light_source);
	frag_to_light_source /= d;
	
	float surface_attitude_factor = dot(light.direction, frag_normal);

	float projection_dp = dot(light.direction, frag_to_light_source);
	if (projection_dp < light.angle.y)
	{
		return vec3(0.);
	}
	
	float distance_attenuation = Calculate_SearchAttenuation(light.angle.x, d);

	float intensity = surface_attitude_factor / distance_attenuation;
	return light.color.rgb * intensity;
}

// support function to calculate the light seen on a given fragment
lowp vec3 BackgroundLightFragment(in highp vec3 frag_direction, in lowp vec3 diffuse)
{
	lowp vec3 beam_illumination = ambient.rgb;
	
	int i = num_point_lights;
	for (int end = i + num_search_lights; i != end; ++ i)
	{
		beam_illumination += LightFragment_SearchBeam(lights[i], frag_direction, far_positive);
	}
	
	return diffuse + beam_illumination;
}

// support function to calculate the light seen on a given fragment
lowp vec3 LightFragment(in highp vec3 frag_position, in highp vec3 frag_normal, in lowp vec3 diffuse)
{
#if defined(TEST_NORMALS)
	return frag_normal;
#endif

	lowp vec3 surface_illumination = ambient.rgb;
	lowp vec3 beam_illumination = vec3(0.);
	
	int i = 0;
	
	for (int end = i + num_point_lights; i != end; ++ i)
	{
		surface_illumination += LightFragment_Point(lights[i], frag_position, frag_normal);
	}
	
	for (int end = i + num_search_lights; i != end; ++ i)
	{
		surface_illumination += LightFragment_Search(lights[i], frag_position, frag_normal);
		
		beam_illumination += LightFragment_SearchBeam(lights[i], frag_position);
	}
	
	return diffuse * surface_illumination + beam_illumination;
}
