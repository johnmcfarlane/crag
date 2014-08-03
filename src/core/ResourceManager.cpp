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

ResourceManager & ResourceManager::Get()
{
	return _singleton;
}

void ResourceManager::Unregister(KeyType const & key)
{
	_mutex.WriteLock();
	
	auto erased = _resources.erase(key);
	if (erased != 1)
	{
		CRAG_DEBUG_DUMP(key);
		DEBUG_BREAK("didn't remove a single element (%d)", int(erased));
	}
	
	_mutex.WriteUnlock();
}

void ResourceManager::Clear()
{
	_mutex.WriteLock();
	DEBUG_MESSAGE("");
	_resources.clear();
	_mutex.WriteUnlock();
}

void ResourceManager::Load(KeyType const & key) const
{
	auto const & resource = GetResource(key);
	resource.Load();
}

void ResourceManager::Unload(KeyType const & key) const
{
	auto const & resource = GetResource(key);
	resource.Unload();
}

ResourceManager::ValueType const & ResourceManager::GetResource(KeyType const & key) const
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

ResourceManager::ValueType & ResourceManager::GetResource(KeyType const & key)
{
	auto const_this = const_cast<ResourceManager const *>(this);
	return const_cast<ValueType &>(const_this->GetResource(key));
}

void ResourceManager::Register(KeyType const & key, ValueType && value)
{
	_mutex.WriteLock();
	
	auto found = _resources.find(key);
	if (found != _resources.end())
	{
		CRAG_DEBUG_DUMP(key);
		CRAG_VERIFY_EQUAL(found->second.GetTypeId(), value.GetTypeId());
		DEBUG_BREAK("multiple resources with same key");
		return;
	}
	
	_resources.insert(std::make_pair(key, std::move(value)));
	
	_mutex.WriteUnlock();
}

ResourceManager::~ResourceManager()
{
	ASSERT(_resources.empty());
}
