//
//  Controller.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace sim
{
	class Entity;

	// a member of Entity which generates distinctive behavior
	class Controller
	{
		OBJECT_NO_COPY(Controller);
	public:
		Controller(Entity & entity);
		virtual ~Controller();

		Entity & GetEntity();
		Entity const & GetEntity() const;
		
		virtual void Tick() = 0;

	private:
		Entity & _entity;
	};
}
