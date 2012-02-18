//
//  Object.h
//  crag
//
//  Created by John McFarlane on 5/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace script
{
	// Base class for objects used by the script daemon.
	// TODO: Extend this system to all daemons.
	class Object
	{
	public:
		// variables
		DEFINE_INTRUSIVE_LIST(Object, List);
		typedef List::const_iterator const_iterator;
		typedef List::iterator iterator;
	};
}
