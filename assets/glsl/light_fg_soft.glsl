//
//  light_fg_soft.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(ENABLE_LIGHTING)

////////////////////////////////////////////////////////////////////////////////
// foreground surfaces with normals (fluffy stuff)

// return light reflected by given point light on a given position
COLOR3 GetPointLightReflection(Light light, VECTOR3 position)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance_sq = dot(to_light, to_light);
	
	return light.color.rgb * .5 / distance_sq;
}

// return light reflected by given search light on a given position
COLOR3 GetSearchLightReflection(Light light, VECTOR3 position)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance = length(to_light);
	VECTOR3 to_light_direction = to_light / distance;
	
	if (dot(to_light_direction, light.direction) < light.angle.y)
	{
		return vec3(0.);
	}
	
	return light.color.rgb * .5 / (distance * distance);
}

// accumulate light reflected and illuminated by given lights on a given position
LightResults ForegroundLight(int point_lights_begin, int point_lights_end, int search_lights_end, VECTOR3 position)
{
	LightResults results = LightResults(COLOR3(0.), COLOR3(0.));

	int i = point_lights_begin;

	for (int num_point_lights = point_lights_end - point_lights_begin; num_point_lights > 0; ++ i, -- num_point_lights)
	{
		Light light = lights[i];

		results.reflection += GetPointLightReflection(light, position);
	}

	for (int num_search_lights = search_lights_end - point_lights_end; num_search_lights > 0; ++ i, -- num_search_lights)
	{
		Light light = lights[i];

		results.reflection += GetSearchLightReflection(light, position);

#if defined(ENABLE_BEAM_LIGHTING)
		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		results.illumination += GetBeamIllumination(light, ray_direction, ray_distance);
#endif
	}

	return results;
}

// return light reflected and illuminated by vertex lights on a given position
LightResults ForegroundLightVertex(VECTOR3 position)
{
	return ForegroundLight(0, vertex_point_lights_end, vertex_search_lights_end, position);
}

// return consolidated light reflected and illuminated by fragment lights on a given position
COLOR4 ForegroundLightFragment(VECTOR3 position, COLOR4 diffuse, LightResults vertex_results)
{
	LightResults results = ForegroundLight(vertex_search_lights_end, fragment_point_lights_end, fragment_search_lights_end, position);
	
	results.reflection += vertex_results.reflection;
	results.illumination += vertex_results.illumination;

	return vec4(ambient.rgb + results.reflection * diffuse.rgb + results.illumination, diffuse.a);
}

#else

// return light reflected and illuminated by vertex lights on a given position
LightResults ForegroundLightVertex(VECTOR3 position)
{
	return LightResults(vec3(0.), vec3(0.));
}

// return consolidated light reflected and illuminated by fragment lights on a given position
COLOR4 ForegroundLightFragment(VECTOR3 position, COLOR4 diffuse, LightResults vertex_results)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING

// return consolidated light reflected and illuminated by all lights on a given position
COLOR4 ForegroundLightAll(VECTOR3 position, COLOR4 diffuse)
{
	LightResults vertex_results = ForegroundLightVertex(position);
	return ForegroundLightFragment(position, diffuse, vertex_results);
}
