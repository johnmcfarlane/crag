//
//  Singleton.h
//  crag
//
//  Created by john on 9/30/09.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


#include "core/debug.h"


namespace core
{
	
	// A singleton simply asserts if it is instanced multiply.
	template <typename S> class Singleton
	{
		OBJECT_NO_COPY (Singleton);
		
	public:
		
		// Singleton is useless on its own so c'tor/d'tor are protected.
		Singleton()
		{
			ASSERT(the_instance == nullptr);
			
			// If S is not derived from Singleton<S>, singleton will be buggy here.
			the_instance = reinterpret_cast<S *>(this);
		}
		
		virtual ~Singleton()
		{
			ASSERT(the_instance == reinterpret_cast<S *>(this));
			the_instance = nullptr;
		}
		
	private:
		
		static S * the_instance;
	};

	template<typename S> S * Singleton<S>::the_instance = nullptr;
}
