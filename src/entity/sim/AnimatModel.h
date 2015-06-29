//
//  entity/sim/AnimatModel.h
//  crag
//
//  Created by john on 2015-06-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <sim/Model.h>

#include "Signal.h"

namespace sim
{
	class AnimatModel final : public Model
	{
		// types
		using super = Model;

	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(AnimatModel);

		AnimatModel(Handle handle, physics::Location const & location) noexcept;

		Receiver & GetHealthReceiver() noexcept;

		void Update() noexcept;

	private:
		// variables
		Receiver _health_receiver;
	};
}
