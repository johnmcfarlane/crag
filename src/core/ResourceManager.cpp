//
//  core/ResourceManager.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-15.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ResourceManager.h"

using namespace crag::core;

////////////////////////////////////////////////////////////////////////////////
// core::ResourceManager member definitions

ResourceManager ResourceManager::_singleton;

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(ResourceManager, self)
	self._mutex.ReadLock();
	for (auto & resource : self._resources)
	{
		CRAG_VERIFY(resource.second);
	}
	self._mutex.ReadUnlock();
CRAG_VERIFY_INVARIANTS_DEFINE_END

ResourceManager & ResourceManager::Get()
{
	return _singleton;
}

void ResourceManager::Unregister(KeyType key)
{
	_mutex.WriteLock();
	
	auto erased = _resources.erase(key);
	if (erased != 1)
	{
		DEBUG_BREAK("didn't remove a single element (%d)", int(erased));
	}
	ASSERT(_resources.find(key) == _resources.end());
	
	_mutex.WriteUnlock();
}

void ResourceManager::Clear()
{
	_mutex.WriteLock();
	_resources.clear();
	_mutex.WriteUnlock();
}

void ResourceManager::Load(KeyType key) const
{
	auto const & resource = GetResource(key);
	resource.Load();
}

void ResourceManager::Unload(KeyType key) const
{
	auto const & resource = GetResource(key);
	resource.Unload();
}

void ResourceManager::UnloadAll() const
{
	_mutex.ReadLock();

	for (auto const & resource_pair : _resources)
	{
		resource_pair.second.Unload();
	}
	
	_mutex.ReadUnlock();
}

ResourceManager::ValueType const & ResourceManager::GetResource(KeyType key) const
{
	_mutex.ReadLock();
	
	auto found = _resources.find(key);
	if (found == _resources.end())
	{
		CRAG_DEBUG_DUMP(key);
		DEBUG_BREAK("failed to find resource");
	}
	
	auto const & resource = found->second;
	
	_mutex.ReadUnlock();
	return resource;
}

ResourceManager::ValueType & ResourceManager::GetResource(KeyType key)
{
	auto const_this = const_cast<ResourceManager const *>(this);
	return const_cast<ValueType &>(const_this->GetResource(key));
}

void ResourceManager::Register(KeyType key, ValueType && value)
{
	_mutex.ReadLock();
	
	auto found = _resources.find(key);
	if (found != _resources.end())
	{
		CRAG_VERIFY_EQUAL(found->second.GetTypeId(), value.GetTypeId());
		DEBUG_BREAK("multiple resources with same key");
		return;
	}
	
	_resources.insert(std::make_pair(key, std::move(value)));
	
	_mutex.ReadUnlock();
}
