/*
 *  Singleton.h
 *  Crag
 *
 *  Created by john on 9/30/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once


#include "core/debug.h"


namespace core
{

	template <typename S> class Singleton
	{
	public:
		static bool IsInstanced()
		{
			return the_instance != nullptr;
		}

		static S * GetPtr()
		{
			return the_instance;
		}

		static S & Get()
		{
			Assert(IsInstanced());
			return * GetPtr();
		}

	protected:
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
