//
//  light_fg_solid.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#define ENABLE_LIGHTING

#if defined(ENABLE_LIGHTING)

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

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

#else

// return light reflected and illuminated by vertex lights on a given surface
void ForegroundLightVertex(in highp vec3 position, in highp vec3 normal, out lowp vec3 reflection, out lowp vec3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
lowp vec4 ForegroundLightFragment(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse, in lowp vec3 reflection, in lowp vec3 illumination)
{
	return diffuse;
}

#endif

// return consolidated light reflected and illuminated by all lights on a given surface
lowp vec4 ForegroundLightAll(in highp vec3 position, in highp vec3 normal, in lowp vec4 diffuse)
{
	lowp vec3 reflection, illumination;
	ForegroundLightVertex(position, normal, reflection, illumination);
	return ForegroundLightFragment(position, normal, diffuse, reflection, illumination);
}
