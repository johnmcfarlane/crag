/*
 *  Singleton.h
 *  Crag
 *
 *  Created by john on 9/30/09.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


#include "core/debug.h"


namespace core
{
	// TODO: Remove this singleton. It's more trouble than it's worth.
	// TODO: (Not to mention that Singletons are considered harmful.)

	// Any class, S, derived from Singleton is guaranteed to have no more than one instance.
	// If multiple objects of class, S, are instanced, the program will assert.
	// Singleton also provides global accessors to the single instance of S.

	// To make your class a singleton, define it as 
	//	class MyClass : public Singleton<MyClass> { ... };
	// or prevent global access, define it as:
	//	class MyClass : private Singleton<MyClass> { ... };
	template <typename S> class Singleton
	{
		OBJECT_NO_COPY (Singleton);
		
	public:
		
		// returns the singleton or nullptr if it is not instanced.
		static S * Ptr()
		{
			return the_instance;
		}

		// returns the singleton; assumes it is instanced. 
		static S & Ref()
		{
			return ref(Ptr());
		}

	protected:
		
		// Singleton is useless on its own so c'tor/d'tor are protected.
		Singleton()
		{
			Assert(the_instance == nullptr);
			
			// If S is not derived from Singleton<S>, singleton will be buggy here.
			the_instance = reinterpret_cast<S *>(this);
		}
		
		virtual ~Singleton()
		{
			Assert(the_instance == reinterpret_cast<S *>(this));
			the_instance = nullptr;
		}

	private:
		
		static S * the_instance;
	};

	template<typename S> S * Singleton<S>::the_instance = nullptr;
}
