//
//  HoverThruster.h
//  crag
//
//  Created by John McFarlane on 2013-03-20.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Thruster.h"

namespace sim
{
	// thruster which applies thrust along the axis of gravity of the entity
	class HoverThruster : public Thruster
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(HoverThruster);

		HoverThruster(Entity & entity, Vector3 const & position, Scalar magnitude);
		~HoverThruster();

	private:
		static void TickThrustDirection(HoverThruster *);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables

		Scalar _magnitude;
	};
}
