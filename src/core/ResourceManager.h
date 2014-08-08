//
//  core/ResourceManager.h
//  crag
//
//  Created by John McFarlane on 2014-01-15.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "HashString.h"
#include "ResourceHandle.h"

#include "smp/ReadersWriterMutex.h"

namespace crag
{
	namespace core
	{
		////////////////////////////////////////////////////////////////////////////////
		// ResourceManager definition - central store for resources; designed for
		// efficient access with option to deallocate and lazily reallocate resources
		// on low memory warning (not implemented)
		
		// Two instances exist: a global singleton-like manager accesible via Get() and
		// a gfx-specific instance for storing all GL objects which need to be
		// recreated after a mobile device resumes the app.

		class ResourceManager final
		{
		public:
			////////////////////////////////////////////////////////////////////////////////
			// types

			using KeyType = HashString;
			using ValueType = Resource;
			using ResourceMap = std::map<KeyType, ValueType>;

			////////////////////////////////////////////////////////////////////////////////
			// functions
		
			static ResourceManager & Get();
			
			// connects a hash string to a function object which creates an object of type, Type
			template <typename Type, typename CreateFunction>
			void Register(KeyType const & key, CreateFunction create_function)
			{
				Register(key, Resource::Create<Type, CreateFunction>(create_function));
			}
			
			// removes resource with given key (invalidating all current handles)
			void Unregister(KeyType const & key);
			
			// as above but indescriminate
			void Clear();

			// returns a permanent handle which can be used to point to desired resource
			template <typename Type>
			ResourceHandle<Type> GetHandle(KeyType const & key) const
			{
				auto const & resource = GetResource(key);
				return ResourceHandle<Type>(& resource);
			}
			
			// ensure that resource exists (necessary for thread-sensitive GL resources)
			void Load(KeyType const & key) const;
			
			// frees resource (it'll get lazily reloaded on subsequent use)
			void Unload(KeyType const & key) const;
			
			// frees all resources
			void UnloadAll() const;

		private:
			Resource const & GetResource(KeyType const & key) const;
			Resource & GetResource(KeyType const & key);
		
			void Register(KeyType const & key, Resource && value);
			
			////////////////////////////////////////////////////////////////////////////////
			// variables

			ResourceMap _resources;
			mutable smp::ReadersWriterMutex _mutex;
			
			static ResourceManager _singleton;
		};
	}
}
