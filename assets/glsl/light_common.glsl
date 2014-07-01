//
//  light_common.glsl
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#define ENABLE_LIGHTING

////////////////////////////////////////////////////////////////////////////////
// light types

// returns (solid) lighting results
struct LightResults
{
	COLOR3 reflection;
	COLOR3 illumination;
};

#if defined(ENABLE_LIGHTING)

// contains information for a single light
struct Light
{
	vec3 position;
	vec3 direction;	// for search lights
	vec4 color;
	vec2 angle;	// for search light, sin/cos
};

////////////////////////////////////////////////////////////////////////////////
// constants

// matches value in src/gfx/Program.h
#define MAX_LIGHTS 6

const float far_positive = + 1000000.;
const float far_negative = - 1000000.;


////////////////////////////////////////////////////////////////////////////////
// uniforms

// light information provided by the renderer
uniform COLOR4 ambient;
uniform Light lights[MAX_LIGHTS];

uniform lowp int vertex_point_lights_end;
uniform lowp int vertex_search_lights_end;
uniform lowp int fragment_point_lights_end;
uniform lowp int fragment_search_lights_end;


////////////////////////////////////////////////////////////////////////////////
// misc types

// used by search beam functions
struct Contact
{
	vec3 position;
	float t;
};


////////////////////////////////////////////////////////////////////////////////
// functions

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

// support function to calculate the light reflected off an idealized atmosphere
// before a given fragment from a given search light
COLOR3 GetBeamIllumination(Light light, VECTOR3 ray_direction, SCALAR ray_distance)
{
	// details of the two contact points between the camera ray and the volume
	// of the search beam; contact1 is behind contact2 (camera-wise)
	Contact contact1, contact2;
	{
		// cone/line intersection test copied from
		// http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
		float AdD = dot(light.direction, ray_direction);
		float cosSqr = Squared(light.angle.y);
		vec3 E = light.position;
		float AdE = dot(light.direction, E);
		float DdE = dot(ray_direction, E);
		float EdE = dot(E, E);
		float c2 = Squared(AdD) - cosSqr;

		float c1 = cosSqr * DdE - AdD * AdE;
		float c0 = AdE * AdE - cosSqr * EdE;

		float discr = Squared(c1) - c0 * c2;
		if (discr <= 0.)
		{
			return vec3(0.);
		}

		// Q(t) = 0 has two distinct real-valued roots.
		float root = sqrt(discr);
		float invC2 = 1. / c2;

		contact1 = SetContact(ray_direction, (- c1 + root) * invC2);
		contact2 = SetContact(ray_direction, (- c1 - root) * invC2);

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
				contact1 = SetContact(ray_direction, far_negative);
			}
		}
		else
		{
			if (good2)
			{
				contact1 = contact2;
				contact2 = SetContact(ray_direction, far_positive);
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
		if (contact2.t > ray_distance)
		{
			contact2.t = ray_distance;
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

#endif	// ENABLE_LIGHTING
