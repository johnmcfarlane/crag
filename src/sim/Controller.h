//
//  Controller.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/counted_object.h"
#include "core/RosterObjectDeclare.h"

namespace sim
{
	class Entity;

	// a member of Entity which generates distinctive behavior
	class Controller : private crag::counted_object<Controller>
	{
		OBJECT_NO_COPY(Controller);
	public:
		Controller(Entity & entity);
		virtual ~Controller();

		CRAG_VERIFY_INVARIANTS_DECLARE(Controller);

		Entity & GetEntity();
		Entity const & GetEntity() const;
	private:
		Entity & _entity;
	};
}
