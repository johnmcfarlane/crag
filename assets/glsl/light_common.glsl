//
//  light_common.glsl
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#define ENABLE_LIGHTING

#if defined(CRAG_USE_GL)
#define ENABLE_BEAM_LIGHTING
#endif

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
	int type;	// [point, search] (or -1 for unused)
};

////////////////////////////////////////////////////////////////////////////////
// constants

const float far_positive = + 1000000.;
const float far_negative = - 1000000.;


////////////////////////////////////////////////////////////////////////////////
// uniforms

// light information provided by the renderer
uniform COLOR4 ambient;

// matches values in src/gfx/Program.h
#define MAX_VERTEX_LIGHTS 6
uniform Light vertex_lights[MAX_VERTEX_LIGHTS];

#if defined(CRAG_USE_GL)
#define MAX_FRAGMENT_LIGHTS 1
uniform Light fragment_lights[MAX_FRAGMENT_LIGHTS];
#endif


////////////////////////////////////////////////////////////////////////////////
// misc types

// used by search beam functions
struct Contact
{
	VECTOR3 position;
	float t;
};


////////////////////////////////////////////////////////////////////////////////
// functions

Contact SetContact(const VECTOR3 camera_direction, const float t)
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

#if defined(ENABLE_BEAM_LIGHTING)

// support function to calculate the light reflected off an idealized atmosphere
// before a given fragment from a given search light
COLOR3 GetBeamIllumination(Light light, VECTOR3 ray_direction, SCALAR ray_distance)
{
	// details of the two contact points between the camera ray and the volume
	// of the search beam; contact1 is behind contact2 (camera-wise)
	SCALAR c1, c2, discr;
	{
		// cone/line intersection test copied from
		// http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
		float AdD = dot(light.direction, ray_direction);
		float cosSqr = Squared(light.angle.y);
		float AdE = dot(light.direction, light.position);
		float DdE = dot(ray_direction, light.position);
		float EdE = dot(light.position, light.position);

		float c0 = AdE * AdE - cosSqr * EdE;
		c1 = cosSqr * DdE - AdD * AdE;
		c2 = Squared(AdD) - cosSqr;

		discr = Squared(c1) - c0 * c2;
		if (discr <= 0.)
		{
			return COLOR3(0.);
		}
	}
	
	float light_distance1, light_distance2;
	{
		// Q(t) = 0 has two distinct real-valued roots.
		float root = sqrt(discr);
		float invC2 = 1. / c2;

		Contact contact1 = SetContact(ray_direction, (- c1 + root) * invC2);
		Contact contact2 = SetContact(ray_direction, (- c1 - root) * invC2);

		bool good1 = GetContactGood(contact1, light);
		bool good2 = GetContactGood(contact2, light);

		if (good1 == good2)
		{
			if (! good1)
			{
				return COLOR3(0.);
			}
		}
		else
		{
			if (good1)
			{
				if (contact1.t > contact2.t)
				{
					contact2 = SetContact(ray_direction, far_positive);
				}
				else
				{
					contact2 = contact1;
					contact1 = SetContact(ray_direction, far_negative);
				}
			}
			else
			{
				if (contact1.t > contact2.t)
				{
					contact1 = SetContact(ray_direction, far_negative);
				}
				else
				{
					contact1 = contact2;
					contact2 = SetContact(ray_direction, far_positive);
				}
			}
		}

		// clip to visible range: [near z, min(far z, distance to surface)]
		contact1.t = max(contact1.t, 0.);
		contact2.t = min(contact2.t, ray_distance);

		if (contact1.t >= contact2.t)
		{
			// beam entirely behind camera or entirely beyond surface
			return COLOR3(0.);
		}
		
		light_distance1 = length(ray_direction * contact1.t - light.position);
		light_distance2 = length(ray_direction * contact2.t - light.position);
	}

	SCALAR x1, x2;
	{
		x1 = pow(light_distance1, -2.);
		x2 = pow(light_distance2, -2.);
	}

	return .25 * light.color.rgb * (x2 + x1);
}

#endif	// ENABLE_BEAM_LIGHTING

#endif	// ENABLE_LIGHTING
