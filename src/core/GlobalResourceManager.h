//
//  core/GlobalResourceManager.h
//  crag
//
//  Created by John McFarlane on 2014-08-23.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ResourceManager.h"

#include "smp/ReadersWriterMutex.h"

namespace crag
{
	// global, cross-thread ResourceManager instance
	class GlobalResourceManager final
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		using Mutex = smp::ReadersWriterMutex;
	public:
		using KeyType = core::ResourceManager::KeyType;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		OBJECT_NO_COPY(GlobalResourceManager);
		GlobalResourceManager() = delete;
	
		// connects a hash string to a function object which creates an object of type, Type
		template <typename Type, typename CreateFunction>
		static void Register(KeyType key, CreateFunction create_function)
		{
			_mutex.WriteLock();
			_singleton.Register<Type, CreateFunction>(key, create_function);
			_mutex.WriteUnlock();
		}
		
		// returns a permanent handle which can be used to point to desired resource
		template <typename Type>
		static core::ResourceHandle<Type> GetHandle(KeyType key)
		{
			_mutex.ReadLock();
			core::ResourceHandle<Type> handle = _singleton.GetHandle<Type>(key);
			_mutex.ReadUnlock();
			return handle;
		}
		
		// as above but indescriminate
		static void Clear()
		{
			_mutex.WriteLock();
			_singleton.Clear();
			_mutex.WriteUnlock();
		}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		static Mutex _mutex;
		static core::ResourceManager _singleton;
	};
}
