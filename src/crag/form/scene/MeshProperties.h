/*
 *  MeshProperties.h
 *  crag
 *
 *  Created by John on 10/21/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace form 
{
	
	// Common info shared between Mesh and MeshBufferObject.
	struct MeshProperties
	{
		typedef Vector3d Vector3;
		
		MeshProperties() 
		: _origin(Vector3::Zero())
		, _num_quaterna(0)
		, _flat_shaded(false)
		{
		}
		
		MeshProperties(Vector3 origin, int num_quaterna, bool flat_shaded) 
		: _origin(origin)
		, _num_quaterna(num_quaterna)
		, _flat_shaded(flat_shaded)
		{
		}
		
		Vector3 _origin;
		int _num_quaterna;
		bool _flat_shaded;
	};
	
}
