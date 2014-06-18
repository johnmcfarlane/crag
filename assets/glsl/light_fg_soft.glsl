//
//  light_fg_soft.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#ifdef GL_ES
precision highp float;
precision highp int;
#endif

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
