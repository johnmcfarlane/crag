//
//  sphere.frag
//  crag
//
//  Created by John McFarlane on 2011-12-29.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from sphere.vert
varying VECTOR4 frag_position;
	
// inputs from the renderer
uniform MATRIX4 projection_matrix;
uniform COLOR4 color;
uniform VECTOR3 center;
uniform SCALAR radius;


// given a ray cast from the origin (eye),
// returns point at which it intersects the sphere given by center/radius.
bool GetIntersection(VECTOR3 ray, out SCALAR t)
{
	SCALAR a = dot(ray, ray);
	SCALAR half_b = dot(ray, center);
	SCALAR c = dot(center, center) - (radius * radius);
	
	SCALAR root = (half_b * half_b) - a * c;
	if (root < 0.)
	{
		// no intersection
		return false;
	}
	root = sqrt(root);

	t = (half_b - root) / a;
	return true;
}


void SetFragmentDepth(vec4 view_position)
{
#ifndef GL_ES
	VECTOR2 clipZW = view_position.z * projection_matrix[2].zw + projection_matrix[3].zw;
	
	gl_FragDepth = 0.5 + 0.5 * clipZW.x / clipZW.y;
#endif
}

void main(void)
{
	SCALAR t;
	if (! GetIntersection(frag_position.xyz, t))
	{
		discard;
	}

	VECTOR4 frag_position = frag_position * t;
	SetFragmentDepth(frag_position);

	VECTOR3 frag_normal = (frag_position.xyz - center) / radius;

	gl_FragColor = ForegroundLightAll(frag_position.xyz, frag_normal.xyz, color);
}
