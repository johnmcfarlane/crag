//
//  Messages.h
//  crag
//
//  Created by John on 2013-02-19.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace gfx
{
	struct NumQuaterneSetMessage
	{
		std::size_t num_quaterne;
	};

	struct FrameDurationSampledMessage
	{
		float frame_duration_ratio;
	};

	struct MeshGenerationPeriodSampledMessage
	{
		core::Time mesh_generation_period;
	};
}
