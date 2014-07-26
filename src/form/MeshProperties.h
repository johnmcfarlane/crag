//
//  MeshProperties.h
//  crag
//
//  Created by John on 10/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Space.h"

namespace geom
{
	class Space;
}

namespace form 
{
	// Common info shared between Mesh and MeshBufferObject.
	struct MeshProperties
	{
		CRAG_VERIFY_INVARIANTS_DECLARE(MeshProperties);

		MeshProperties() 
		: _num_quaterne(0)
		{
		}
		
		MeshProperties(geom::Space const & space, int num_quaterne) 
		: _space(space)
		, _num_quaterne(num_quaterne)
		{
		}
		
		geom::Space _space;
		std::size_t _num_quaterne;
	};

#if defined(CRAG_VERIFY_ENABLED)
	inline CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(MeshProperties, self)
		CRAG_VERIFY(self._space);
	CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif
}
