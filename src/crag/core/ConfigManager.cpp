/*
 *  ConfigManager.cpp
 *  Crag
 *
 *  Created by john on 6/26/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "core/ConfigManager.h"
#include "core/ConfigEntry.h"

#if defined(ENABLE_CONFIG)

#include "core/debug.h"


namespace ANONYMOUS {

ConfigEntry * list_head = nullptr;

int const max_string_size = 1024;

bool parameter_set_complete = false;

}


//////////////////////////////////////////////////////////////////////
// Config function definitions

void ConfigManager::Load(std::istream & in)
{
	parameter_set_complete = true;
	
	char line[max_string_size];
	for (int line_num = 0; in.getline(line, max_string_size - 1) != nullptr; ++ line_num)
	{
		char * value_string = strchr(line, '=');
		if (value_string != nullptr)
		{
			* (value_string ++) = '\0';
			char const * name = line;
			ConfigEntry * parameter = GetParameter(name);
			if (parameter != nullptr)
			{
				parameter->StringToValue(value_string);
			}
			else
			{
				std::cerr << "ConfigManager: unrecognised parameter " << name << " on line " << line_num << ".\n";
			}
		}
	}
}

void ConfigManager::Save(std::ostream & out)
{
	parameter_set_complete = true;
	
	for (ConfigEntry const * iterator = list_head; iterator != nullptr; iterator = iterator->next)
	{
		char value_string[max_string_size];
		iterator->ValueToString(value_string);
		out << iterator->name << "=" << value_string << '\n';
	}
}

void ConfigManager::AddParameter(ConfigEntry & parameter)
{
	// TODO: maybe alpha-sort
	Assert(GetParameter(parameter.name) == nullptr);
	Assert(! parameter_set_complete);

	parameter.next = list_head;
	list_head = & parameter;
}

ConfigEntry * ConfigManager::GetParameter(char const * name)
{
	int string_length = strlen(name);
	for (ConfigEntry * iterator = list_head; iterator != nullptr; iterator = iterator->next)
	{
		// TODO: Isn't there a problem here if one string is a sub-string of the other?
		if (strncmp(name, iterator->name, string_length) == 0)
		{
			return iterator;
		}
	}
	return nullptr;
}

#else

void ConfigManager::Load(std::istream &)
{
}

void ConfigManager::Save(std::ostream &)
{
}

#endif	// ENABLE_CONFIG
