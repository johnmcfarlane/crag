//
//  core/GlobalResourceManager.cpp
//  crag
//
//  Created by John McFarlane on 2014-08-23.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "GlobalResourceManager.h"

using namespace crag;

GlobalResourceManager::Mutex GlobalResourceManager::_mutex;
crag::core::ResourceManager GlobalResourceManager::_singleton;
