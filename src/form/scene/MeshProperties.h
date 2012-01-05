//
//  MeshProperties.h
//  crag
//
//  Created by John on 10/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"


namespace form 
{
	
	// Common info shared between Mesh and MeshBufferObject.
	struct MeshProperties
	{
		typedef sim::Vector3 Vector;
		
		MeshProperties() 
		: _origin(Vector::Zero())
		, _num_quaterne(0)
		, _flat_shaded(false)
		{
		}
		
		MeshProperties(Vector origin, int num_quaterne, bool flat_shaded) 
		: _origin(origin)
		, _num_quaterne(num_quaterne)
		, _flat_shaded(flat_shaded)
		{
		}
		
		Vector _origin;
		int _num_quaterne;
		bool _flat_shaded;
	};
	
}
