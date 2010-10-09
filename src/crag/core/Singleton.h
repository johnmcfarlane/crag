/*
 *  Singleton.h
 *  Crag
 *
 *  Created by john on 9/30/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


#include "core/debug.h"


namespace core
{

	// Any class, S, derived from Singleton is guaranteed to have no more than one instance.
	// If multiple objects of class, S, are instanced, the program will assert.
	// Singleton also provides global accessors to the single instance of S.

	// To make your class a singleton, define it as: 
	//	class MyClass : public Singleton<MyClass> { ... };
	template <typename S> class Singleton
	{
		OBJECT_NO_COPY (Singleton);
		
	public:
		
		// true iff the instance of S exists.
		static bool IsInstanced()
		{
			return the_instance != nullptr;
		}

		// returns the singleton or nullptr if it is not instanced.
		static S * GetPtr()
		{
			return the_instance;
		}

		// returns the singleton; assumes it is instanced. 
		static S & Get()
		{
			Assert(IsInstanced());
			return * GetPtr();
		}

	protected:
		
		// Singleton is useless on its own so c'tor/d'tor are protected.
		Singleton()
		{
			Assert(the_instance == nullptr);
			
			// If S is not derived from Singleton<S>, singleton will break here.
			the_instance = reinterpret_cast<S *>(this);
		}
		
		virtual ~Singleton()
		{
			Assert(the_instance == this);
			the_instance = nullptr;
		}

	private:
		
		static S * the_instance;
	};

	template<typename T> T * core::Singleton<T>::the_instance = nullptr;
}
