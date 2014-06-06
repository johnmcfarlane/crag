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

const float far_positive = 1000000.;
const float far_negative = - far_positive;

// contains information for a single light
struct Light
{
	vec3 position;
	vec3 direction;	// for search lights
	vec4 color;
	vec2 angle;	// for search light, sin/cos
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

Contact SetContact(const vec3 camera_direction, const float t)
{
	Contact contact;
	contact.position = camera_direction * t;
	contact.t = t;
	return contact;
}

bool GetContactGood(const Contact contact, const Light light)
{
	return dot(contact.position - light.position, light.direction) < 0.;
}

vec3 LightFragment_SearchBeam(const Light light, const highp vec3 camera_direction, const float camera_distance)
{
	// details of the two contact points between the camera ray and the volume
	// of the search beam; contact1 is behind contact2 (camera-wise)
	Contact contact1, contact2;
	{
		// cone/line intersection test copied from
		// http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
		float AdD = dot(light.direction, camera_direction);
		float cosSqr = Squared(light.angle.y);
		vec3 E = - light.position;
		float AdE = dot(light.direction, E);
		float DdE = dot(camera_direction, E);
		float EdE = dot(E, E);
		float c2 = cosSqr - Squared(AdD);

		float c1 = AdD * AdE - cosSqr * DdE;
		float c0 = AdE * AdE - cosSqr * EdE;

		float discr = Squared(c1) + c0 * c2;
		if (discr <= 0.)
		{
			return vec3(0.);
		}

		// Q(t) = 0 has two distinct real-valued roots.
		float root = sqrt(discr);
		float invC2 = -1. / c2;

		contact1 = SetContact(camera_direction, (- c1 + root) * invC2);
		contact2 = SetContact(camera_direction, (- c1 - root) * invC2);

		// not tested when beam is in behind camera
		if (contact1.t > contact2.t)
		{
			Contact tmp = contact1;
			contact1 = contact2;
			contact2 = tmp;
		}
	
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
				contact2 = contact1;
				contact1 = SetContact(camera_direction, far_negative);
			}
		}
		else
		{
			if (good2)
			{
				contact1 = contact2;
				contact2 = SetContact(camera_direction, far_positive);
			}
			else
			{
				return vec3(0.);
			}
		}

		// clip to near z
		if (contact1.t < 0.)
		{
			contact1.t = 0.;
		}
		
		// clip to solid surface or far z
		if (contact2.t > camera_distance)
		{
			contact2.t = camera_distance;
		}

		if (contact1.t >= contact2.t)
		{
			// beam entirely behind camera or entirely beyond surface
			return vec3(0.);
		}
	}
	
	{
		float depth = contact2.t - contact1.t;
		float light_distance1 = length(contact1.position - light.position);
		float light_distance2 = length(contact2.position - light.position);
		
		float x1 = pow(light_distance1, -2.);
		float x2 = pow(light_distance2, -2.);

		return .25 * light.color.rgb * (x2 + x1);
	}
}

vec3 LightFragment_SearchBeam(const Light light, const highp vec3 frag_position)
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
	highp vec3 frag_to_light = light.position - frag_position;
	highp float distance = length(frag_to_light);
	highp vec3 frag_to_light_direction = frag_to_light / distance;
	
	if (dot(frag_to_light_direction, light.direction) < light.angle.y)
	{
		return vec3(0.);
	}
	
	highp float dp = dot(frag_to_light_direction, frag_normal);
	highp float attenuation = max(dp / (distance * distance), 0.0);

	lowp vec3 color = light.color.rgb * attenuation;
	
	return color;
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
