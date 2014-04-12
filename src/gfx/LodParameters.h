//
//  gfx/LodParameters.h
//  crag
//
//  Created by John on 2014/03/30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace gfx
{
	// LODding algorithms require this information to determine the LOD of 
	// visual features.
	struct LodParameters
	{
		CRAG_VERIFY_INVARIANTS_DECLARE(LodParameters);
		// the camera position or some other subject of 'focus'
		// with minimum range within which detail does not increase
		Vector3 center;
		Scalar min_distance;
	};

#if defined(CRAG_VERIFY_ENABLED)
	inline CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(LodParameters, self)
		CRAG_VERIFY(self.center);
		CRAG_VERIFY(self.min_distance);
	CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif
}
