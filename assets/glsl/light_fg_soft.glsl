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
void ForegroundLight(Light light, VECTOR3 position, inout LightResults results)
{
	if (light.type == 0)
	{
		results.reflection += GetPointLightReflection(light, position);
	}
	else if (light.type == 1)
	{
		results.reflection += GetSearchLightReflection(light, position);

#if defined(ENABLE_BEAM_LIGHTING)
		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		results.illumination += GetBeamIllumination(light, ray_direction, ray_distance);
#endif
	}
}

// return light reflected and illuminated by vertex lights on a given position
LightResults ForegroundLightVertex(VECTOR3 position)
{
	LightResults results = LightResults(COLOR3(0.), COLOR3(0.));

	for (int i = 0; i < MAX_VERTEX_LIGHTS; ++ i)
	{
		ForegroundLight(vertex_lights[i], position, results);
	}
	
	return results;
}

// return consolidated light reflected and illuminated by fragment lights on a given position
COLOR4 ForegroundLightFragment(VECTOR3 position, COLOR4 diffuse, LightResults vertex_results)
{
	LightResults results;
	results.reflection = vertex_results.reflection;
	results.illumination = vertex_results.illumination;
	
#if defined(MAX_FRAGMENT_LIGHTS)
	for (int i = 0; i < MAX_FRAGMENT_LIGHTS; ++ i)
	{
		ForegroundLight(fragment_lights[i], position, results);
	}
#endif

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
