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
COLOR3 GetPointLightReflection(in Light light, in VECTOR3 position)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance_sq = dot(to_light, to_light);
	
	return light.color.rgb * .5 / distance_sq;
}

// return light reflected by given search light on a given position
COLOR3 GetSearchLightReflection(in Light light, in VECTOR3 position)
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
void ForegroundLight(const in ResolutionLights resolution_lights, in VECTOR3 position, inout COLOR3 reflection, inout COLOR3 illumination)
{
	for (int i = resolution_lights.point_lights.num_lights; i -- > 0;)
	{
		reflection += GetPointLightReflection(resolution_lights.point_lights.lights[i], position);
	}

	for (int i = resolution_lights.search_lights.num_lights; i -- > 0;)
	{
		reflection += GetSearchLightReflection(resolution_lights.search_lights.lights[i], position);

		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		illumination += GetBeamIllumination(resolution_lights.search_lights.lights[i], ray_direction, ray_distance);
	}
}

// return light reflected and illuminated by vertex lights on a given position
void ForegroundLightVertex(in VECTOR3 position, out COLOR3 reflection, out COLOR3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
	ForegroundLight(vertex_lights, position, reflection, illumination);
}

// return consolidated light reflected and illuminated by fragment lights on a given position
COLOR4 ForegroundLightFragment(in VECTOR3 position, in COLOR4 diffuse, in COLOR3 reflection, in COLOR3 illumination)
{
	ForegroundLight(fragment_lights, position, reflection, illumination);
	return vec4(ambient.rgb + reflection * diffuse.rgb + illumination, diffuse.a);
}

#else

// return light reflected and illuminated by vertex lights on a given position
void ForegroundLightVertex(in VECTOR3 position, out COLOR3 reflection, out COLOR3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
}

// return consolidated light reflected and illuminated by fragment lights on a given position
COLOR4 ForegroundLightFragment(in VECTOR3 position, in COLOR4 diffuse, in COLOR3 reflection, in COLOR3 illumination)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING

// return consolidated light reflected and illuminated by all lights on a given position
COLOR4 ForegroundLightAll(in VECTOR3 position, in COLOR4 diffuse)
{
	COLOR3 reflection, illumination;
	ForegroundLightVertex(position, reflection, illumination);
	return ForegroundLightFragment(position, diffuse, reflection, illumination);
}
