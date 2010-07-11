/*
 *  FormationFunctor.h
 *  crag
 *
 *  Created by John on 6/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"


namespace form
{
	// forward declaration
	class Formation;
	class Polyhedron;

	// Formation Functor Class
	// Can be passed to ForEachFormation function to iterate over formations (and its model) 
	// in a thread-safe way, i.e. won't get called while nodes are being created or destroyed.
	class FormationFunctor
	{
	public:
		virtual ~FormationFunctor() { }

		virtual void SetSceneOrigin(sim::Vector3 const & scene_origin) = 0;
		virtual void operator()(Formation const & formation, Polyhedron const & model) = 0;
	};
}
