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
		GlobalResourceManager();
		~GlobalResourceManager();
	
		// connects a hash string to a function object which creates an object of type, Type
		template <typename Type, typename CreateFunction>
		static void Register(KeyType key, CreateFunction create_function)
		{
			_singleton->_mutex.WriteLock();
			_singleton->_resource_manager.Register<Type, CreateFunction>(key, create_function);
			_singleton->_mutex.WriteUnlock();
		}
		
		// returns a permanent handle which can be used to point to desired resource
		template <typename Type>
		static core::ResourceHandle<Type> GetHandle(KeyType key)
		{
			_singleton->_mutex.ReadLock();
			core::ResourceHandle<Type> handle = _singleton->_resource_manager.GetHandle<Type>(key);
			_singleton->_mutex.ReadUnlock();
			return handle;
		}
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Mutex _mutex;
		core::ResourceManager _resource_manager;

		static GlobalResourceManager * _singleton;
	};
}
