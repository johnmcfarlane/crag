//
//  MeshProperties.h
//  crag
//
//  Created by John on 10/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/origin.h"

namespace form 
{
	
	// Common info shared between Mesh and MeshBufferObject.
	struct MeshProperties
	{
		typedef geom::abs::Vector3 Vector;
		
#if defined(VERIFY)
		void Verify() const
		{
			VerifyObject(_origin);
			VerifyOp(_num_quaterne, >=, 0);
		}
#endif

		MeshProperties() 
		: _origin(Vector::Zero())
		, _num_quaterne(0)
		{
		}
		
		MeshProperties(Vector origin, int num_quaterne) 
		: _origin(origin)
		, _num_quaterne(num_quaterne)
		{
		}
		
		Vector _origin;
		int _num_quaterne;
	};
	
}
