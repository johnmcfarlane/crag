/*
 *  ConfigManager.h
 *  Crag
 *
 *  Created by john on 6/26/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "ConfigEntry.h"


namespace core
{

	// Contains the complete list of configuration settings.
	class ConfigManager
	{
	public:
		ConfigManager();
		~ConfigManager();
		
	private:
		bool Load();
		void Save();
	};

}
