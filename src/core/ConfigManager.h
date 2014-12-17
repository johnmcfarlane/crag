//
//  ConfigManager.h
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ConfigEntry.h"

namespace crag {
namespace core {
namespace config
{
	// Contains the complete list of configuration settings.
	class Manager
	{
	public:
		Manager();
		~Manager();

		bool ParseCommandLine(int argc, char * const * argv);
	private:
		bool Load();
		void Save();
	};

}}}
