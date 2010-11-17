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

#include "Enumeration.h"


namespace core
{
	
	// forward-declaration
	class ConfigEntry;


	// Contains the complete list of configuration settings.
	class ConfigManager : public core::Enumeration<ConfigEntry>
	{
	public:
		ConfigManager();
		~ConfigManager();
		
	private:
		bool Load();
		void Save();
	};

}
