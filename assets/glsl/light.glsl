//
//  light.glsl
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#define ENABLE_LIGHTING

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

////////////////////////////////////////////////////////////////////////////////
// constants

// matches value in src/gfx/Program.h
const int max_attribute_lights = 6;

const float far_positive = 1000000.;
const float far_negative = - far_positive;


////////////////////////////////////////////////////////////////////////////////
// light types / variables

// contains information for a single light
struct Light
{
	vec3 position;
	vec3 direction;	// for search lights
	vec4 color;
	vec2 angle;	// for search light, sin/cos
};

struct TypeLights
{
	Light lights[max_attribute_lights];
	int num_lights;
};

struct ResolutionLights
{
	TypeLights types[2];	// point / search
};

struct Lights
{
	ResolutionLights resolutions[2];	// vertex, fragment
};

// light information provided by the renderer
uniform vec4 ambient;
uniform Lights lights;


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
lowp vec3 GetBeamIllumination(const Light light, const highp vec3 ray_direction, const float ray_distance)
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// top-level lighting functions

#if defined(ENABLE_LIGHTING)

////////////////////////////////////////////////////////////////////////////////
// foreground surfaces with normals (solids)

// return light reflected by given point light on a given surface
lowp vec3 GetPointLightReflection(in Light light, in highp vec3 position, in highp vec3 normal)
{
	highp vec3 to_light = light.position - position;
	highp float distance = length(to_light);
	
	highp float dp = dot(to_light, normal);
	highp float attenuation = max(dp / (distance * distance * distance), 0.0);

	return light.color.rgb * attenuation;
}

// return light reflected by given search light on a given surface
lowp vec3 GetSearchLightReflection(in Light light, in highp vec3 position, in highp vec3 normal)
{
	highp vec3 to_light = light.position - position;
	highp float distance = length(to_light);
	highp vec3 to_light_direction = to_light / distance;
	
	if (dot(to_light_direction, light.direction) < light.angle.y)
	{
		return vec3(0.);
	}
	
	highp float dp = dot(to_light_direction, normal);
	highp float attenuation = max(dp / (distance * distance), 0.0);

	lowp vec3 color = light.color.rgb * attenuation;
	
	return color;
}

// accumulate light reflected and illuminated by given lights on a given surface
void ForegroundLight(const in ResolutionLights resolution_lights, in highp vec3 position, in highp vec3 normal, inout lowp vec3 reflection, inout lowp vec3 illumination)
{
	for (int i = 0, num_point_lights = resolution_lights.types[0].num_lights; i != num_point_lights; ++ i)
	{
		reflection += GetPointLightReflection(resolution_lights.types[0].lights[i], position, normal);
	}

	for (int i = 0, num_search_lights = resolution_lights.types[1].num_lights; i != num_search_lights; ++ i)
	{
		reflection += GetSearchLightReflection(resolution_lights.types[1].lights[i], position, normal);

		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		illumination += GetBeamIllumination(resolution_lights.types[1].lights[i], ray_direction, ray_distance);
	}
}

// return light reflected and illuminated by vertex lights on a given surface
void ForegroundLightVertex(in highp vec3 position, in highp vec3 normal, out lowp vec3 reflection, out lowp vec3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
	ForegroundLight(lights.resolutions[0], position, normal, reflection, illumination);
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
lowp vec4 ForegroundLightFragment(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse, in lowp vec3 reflection, in lowp vec3 illumination)
{
	ForegroundLight(lights.resolutions[1], position, normal, reflection, illumination);
	return vec4(ambient.rgb + reflection * diffuse.rgb + illumination, diffuse.a);
}

// return consolidated light reflected and illuminated by all lights on a given surface
lowp vec4 ForegroundLightAll(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse)
{
	lowp vec3 reflection, illumination;
	ForegroundLightVertex(position, normal, reflection, illumination);
	return ForegroundLightFragment(position, normal, diffuse, reflection, illumination);
}

////////////////////////////////////////////////////////////////////////////////
// foreground surfaces with normals (fluffy stuff)

// return light reflected by given point light on a given position
lowp vec3 GetPointLightReflection(in Light light, in highp vec3 position)
{
	highp vec3 to_light = light.position - position;
	highp float distance_sq = dot(to_light, to_light);
	
	return light.color.rgb * .5 / distance_sq;
}

// return light reflected by given search light on a given position
lowp vec3 GetSearchLightReflection(in Light light, in highp vec3 position)
{
	highp vec3 to_light = light.position - position;
	highp float distance = length(to_light);
	highp vec3 to_light_direction = to_light / distance;
	
	if (dot(to_light_direction, light.direction) < light.angle.y)
	{
		return vec3(0.);
	}
	
	return light.color.rgb * .5 / (distance * distance);
}

// accumulate light reflected and illuminated by given lights on a given position
void ForegroundLight(const in ResolutionLights resolution_lights, in highp vec3 position, inout lowp vec3 reflection, inout lowp vec3 illumination)
{
	for (int i = 0, num_point_lights = resolution_lights.types[0].num_lights; i != num_point_lights; ++ i)
	{
		reflection += GetPointLightReflection(resolution_lights.types[0].lights[i], position);
	}

	for (int i = 0, num_search_lights = resolution_lights.types[1].num_lights; i != num_search_lights; ++ i)
	{
		reflection += GetSearchLightReflection(resolution_lights.types[1].lights[i], position);

		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		illumination += GetBeamIllumination(resolution_lights.types[1].lights[i], ray_direction, ray_distance);
	}
}

// return light reflected and illuminated by vertex lights on a given position
void ForegroundLightVertex(in highp vec3 position, out lowp vec3 reflection, out lowp vec3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
	ForegroundLight(lights.resolutions[0], position, reflection, illumination);
}

// return consolidated light reflected and illuminated by fragment lights on a given position
lowp vec4 ForegroundLightFragment(in highp vec3 position, in lowp vec4 diffuse, in lowp vec3 reflection, in lowp vec3 illumination)
{
	ForegroundLight(lights.resolutions[1], position, reflection, illumination);
	return vec4(ambient.rgb + reflection * diffuse.rgb + illumination, diffuse.a);
}

// return consolidated light reflected and illuminated by all lights on a given position
lowp vec4 ForegroundLightAll(in highp vec3 position, in lowp vec4 diffuse)
{
	lowp vec3 reflection, illumination;
	ForegroundLightVertex(position, reflection, illumination);
	return ForegroundLightFragment(position, diffuse, reflection, illumination);
}

////////////////////////////////////////////////////////////////////////////////
// background surfaces (skybox)

// return consolidated light reflected and illuminated by given lights from a given direction
void BackgroundLight(in TypeLights search_lights, in highp vec3 ray_direction, inout lowp vec3 illumination)
{
	for (int i = 0, num_search_lights = search_lights.num_lights; i != num_search_lights; ++ i)
	{
		illumination += GetBeamIllumination(search_lights.lights[i], ray_direction, far_positive);
	}
}

// return consolidated light reflected and illuminated by all lights from a given direction
lowp vec3 BackgroundLightFragment(in highp vec3 ray_direction, in lowp vec3 diffuse)
{
	lowp vec3 color = diffuse;
	BackgroundLight(lights.resolutions[0].types[1], ray_direction, color);
	BackgroundLight(lights.resolutions[1].types[1], ray_direction, color);
	return color;
}

#else

// get color value of foreground fragment
void ForegroundLightVertex(in highp vec3 position, in highp vec3 normal, out lowp vec3 reflection, out lowp vec3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
}

// get color value of foreground fragment
lowp vec4 ForegroundLightFragment(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse, in lowp vec3 reflection, in lowp vec3 illumination)
{
	return diffuse;
}

// combined lighting of ForegroundLightVertex and ForegroundLightFragment
lowp vec4 ForegroundLightAll(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse)
{
	return diffuse;
}

// combined lighting of ForegroundLightVertex and ForegroundLightFragment
lowp vec4 ForegroundLightAll(in highp vec3 position, in lowp vec4 diffuse)
{
	return diffuse;
}

// get color value of background (skybox) fragment
lowp vec3 BackgroundLightFragment(in highp vec3 ray_direction, in lowp vec3 diffuse)
{
	return diffuse;
}

#endif
