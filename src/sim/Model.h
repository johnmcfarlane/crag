//
//  sim/Model.h
//  crag
//
//  Created by john on 2015-06-25.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <ipc/Handle.h>

#include <core/RosterObjectDeclare.h>

namespace gfx
{
	DECLARE_CLASS_HANDLE(Object);	// gfx::ObjectHandle
}

namespace physics
{
	class Location;
}

namespace sim
{
	class Model final
	{
	public:
		using Handle = gfx::ObjectHandle;

		CRAG_ROSTER_OBJECT_DECLARE(Model);

		~Model();
		Model(Handle handle, physics::Location const & location);

		Handle GetHandle() const;

		void Update();

	private:
		gfx::ObjectHandle _handle;
		physics::Location const & _location;
	};
}
