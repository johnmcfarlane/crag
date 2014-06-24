//
//  light_fg_solid.glsl
//  crag
//
//  Created by John McFarlane on 2014-06-17.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2014 John McFarlane. All rights reserved.
//

#if defined(ENABLE_LIGHTING)

////////////////////////////////////////////////////////////////////////////////
// foreground surfaces with normals (solids)

// return light reflected by given point light on a given surface
COLOR3 GetPointLightReflection(const Light light, in VECTOR3 position, in VECTOR3 normal)
{
	VECTOR3 to_light = light.position - position;
	SCALAR distance = length(to_light);
	
	SCALAR dp = dot(to_light, normal);
	SCALAR attenuation = max(dp / (distance * distance * distance), 0.0);

	return light.color.rgb * attenuation;
}

// return light reflected by given search light on a given surface
COLOR3 GetSearchLightReflection(const Light light, in VECTOR3 position, in VECTOR3 normal)
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

	COLOR3 color = light.color.rgb * attenuation;
	
	return color;
}

// accumulate light reflected and illuminated by given lights on a given surface
void ForegroundLight(const in ResolutionLights resolution_lights, in VECTOR3 position, in VECTOR3 normal, inout COLOR3 reflection, inout COLOR3 illumination)
{
	for (int i = resolution_lights.types[0].num_lights; i -- > 0;)
	{
		reflection += GetPointLightReflection(resolution_lights.types[0].lights[i], position, normal);
	}

	for (int i = resolution_lights.types[1].num_lights; i -- > 0;)
	{
		reflection += GetSearchLightReflection(resolution_lights.types[1].lights[i], position, normal);

		float ray_distance = length(position);
		vec3 ray_direction = position / ray_distance;
		illumination += GetBeamIllumination(resolution_lights.types[1].lights[i], ray_direction, ray_distance);
	}
}

// return light reflected and illuminated by vertex lights on a given surface
void ForegroundLightVertex(in VECTOR3 position, in VECTOR3 normal, out COLOR3 reflection, out COLOR3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
	ForegroundLight(lights.resolutions[0], position, normal, reflection, illumination);
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
COLOR4 ForegroundLightFragment(in VECTOR3 position, in VECTOR3 normal, in COLOR4 diffuse, in COLOR3 reflection, in COLOR3 illumination)
{
	ForegroundLight(lights.resolutions[1], position, normal, reflection, illumination);
	return vec4(ambient.rgb + reflection * diffuse.rgb + illumination, diffuse.a);
}

#else

// return light reflected and illuminated by vertex lights on a given surface
void ForegroundLightVertex(in VECTOR3 position, in VECTOR3 normal, out COLOR3 reflection, out COLOR3 illumination)
{
	reflection = vec3(0.);
	illumination = vec3(0.);
}

// return consolidated light reflected and illuminated by fragment lights on a given surface
COLOR4 ForegroundLightFragment(in VECTOR3 position, in VECTOR3 normal, in COLOR4 diffuse, in COLOR3 reflection, in COLOR3 illumination)
{
	return diffuse;
}

#endif	// ENABLE_LIGHTING

// return consolidated light reflected and illuminated by all lights on a given surface
COLOR4 ForegroundLightAll(in VECTOR3 position, in VECTOR3 normal, in COLOR4 diffuse)
{
	COLOR3 reflection, illumination;
	ForegroundLightVertex(position, normal, reflection, illumination);
	return ForegroundLightFragment(position, normal, diffuse, reflection, illumination);
}
