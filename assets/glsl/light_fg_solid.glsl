//
//  light_fg_solid.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(ENABLE_LIGHTING)

#if defined(GL_ES)
#define LIGHT3 VECTOR3
#else
#define LIGHT3 COLOR3
#endif

////////////////////////////////////////////////////////////////////////////////
// foreground surfaces with normals (solids)

// return light reflected by given point light on a given surface
LIGHT3 GetPointLightReflection(Light light, VECTOR3 position, VECTOR3 normal)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance = length(to_light);
	VECTOR3 to_light_direction = to_light / distance;
	
	SCALAR dp = dot(to_light_direction, normal);
	SCALAR attenuation = max(dp / (distance * distance), 0.0);

	return LIGHT3(light.color.rgb) * attenuation;
}

// return light reflected by given search light on a given surface
LIGHT3 GetSearchLightReflection(const Light light, VECTOR3 position, VECTOR3 normal)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance = length(to_light);
	VECTOR3 to_light_direction = to_light / distance;
	
	if (dot(to_light_direction, light.direction) < light.angle.y)
	{
		return vec3(0.);
	}
	
	SCALAR dp = dot(to_light_direction, normal);
	SCALAR attenuation = max(dp / (distance * distance), 0.0);

	return LIGHT3(light.color.rgb) * attenuation;
}

// accumulate light reflected and illuminated by given lights on a given surface
LightResults ForegroundLight(int point_lights_begin, int point_lights_end, int search_lights_end, VECTOR3 position, VECTOR3 normal)
{
	LightResults results = LightResults(COLOR3(0.), COLOR3(0.));

	int i = point_lights_begin;

	for (int i = point_lights_begin; i != point_lights_end; ++ i)
	{
		Light light = lights[i];

		results.reflection += GetPointLightReflection(light, position, normal);
	}

	for (int i = point_lights_end; i != search_lights_end; ++ i)
	{
		Light light = lights[i];

		results.reflection += GetSearchLightReflection(light, position, normal);

#if defined(ENABLE_BEAM_LIGHTING)
		SCALAR ray_distance = length(position);
		VECTOR3 ray_direction = position / ray_distance;
		results.illumination += GetBeamIllumination(light, ray_direction, ray_distance);
#endif
	}

	return results;
}

// return light reflected and illuminated by vertex lights on a given surface
LightResults ForegroundLightVertex(VECTOR3 position, VECTOR3 normal)
{
	return ForegroundLight(0, vertex_point_lights_end, vertex_search_lights_end, position, normal);
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
COLOR4 ForegroundLightFragment(VECTOR3 position, VECTOR3 normal, COLOR4 diffuse, COLOR3 reflection, COLOR3 illumination)
{
	LightResults results = ForegroundLight(vertex_search_lights_end, fragment_point_lights_end, fragment_search_lights_end, position, normal);
	results.reflection += reflection;
	results.illumination += illumination;
	return COLOR4(ambient.rgb + results.reflection * diffuse.rgb + results.illumination, diffuse.a);
}

#else

// return light reflected and illuminated by vertex lights on a given surface
LightResults ForegroundLightVertex(VECTOR3 position, VECTOR3 normal)
{
	return LightResults(vec3(0.), vec3(0.));
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
COLOR4 ForegroundLightFragment(VECTOR3 position, VECTOR3 normal, COLOR4 diffuse, COLOR3 reflection, COLOR3 illumination)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING

// return consolidated light reflected and illuminated by all lights on a given surface
COLOR4 ForegroundLightAll(VECTOR3 position, VECTOR3 normal, COLOR4 diffuse)
{
	LightResults vertex_results = ForegroundLightVertex(position, normal);
	return ForegroundLightFragment(position, normal, diffuse, vertex_results.reflection, vertex_results.illumination);
}
