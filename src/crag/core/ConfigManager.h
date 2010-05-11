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

class ConfigEntry;

class ConfigManager
{
public:
	static void AddParameter(ConfigEntry & parameter);

	static void Load(std::istream & in);
	static void Save(std::ostream & out);

private:
	static ConfigEntry * GetParameter(char const * name);
};
