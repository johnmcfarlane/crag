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
		
		CRAG_VERIFY_INVARIANTS_DECLARE(MeshProperties);

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
		std::size_t _num_quaterne;
	};

#if defined(CRAG_VERIFY_ENABLED)
	inline CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(MeshProperties, self)
		CRAG_VERIFY(self._origin);
	CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif
}
