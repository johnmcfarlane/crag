/*
 *  Singleton.h
 *  crag
 *
 *  Created by John McFarlane on 1/31/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Mutex.h"
#include "ptr.h"

#include "core/Singleton.h"


namespace smp
{
	
	// Thread-safe wrapper for singleton.
	template <typename S> class Singleton : protected core::Singleton<S>
	{
	public:
		static ptr<S> GetPtr()
		{
			S & ref = core::Singleton<S>::Get();
			return ptr<S>(ref, ref._mutex);
		}
		
	private:
		Mutex _mutex;
	};
}
