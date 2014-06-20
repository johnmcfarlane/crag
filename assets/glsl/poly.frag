//
//  poly_f.frag
//  crag
//
//  Created by John McFarlane on 2012-01-08.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

// inputs from poly_f.vert
varying VECTOR3 fragment_position;
varying VECTOR3 fragment_normal;
varying COLOR4 fragment_diffuse;
varying COLOR3 fragment_reflection;
varying COLOR3 fragment_illumination;

void main(void)
{
	VECTOR3 normal = normalize(fragment_normal);

	gl_FragColor = ForegroundLightFragment(
		fragment_position, 
		fragment_normal, 
		fragment_diffuse, 
		fragment_reflection, 
		fragment_illumination);
}
