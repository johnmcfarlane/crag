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

#include <fstream>


namespace 
{

	ConfigEntry * list_head = nullptr;

	int const max_string_size = 1024;

	bool parameter_set_complete = false;
	bool defaults_file_ood = false;

	char const * config_filename = "crag.cfg";
	char const * defaults_filename = "default.cfg";
}


//////////////////////////////////////////////////////////////////////
// Config function definitions

bool ConfigManager::Load()
{
	// open files
	std::ifstream config_file(config_filename);
	if (! config_file.is_open())
	{
		std::cout << "Failed to open config file, " << config_filename << ".\n";
		return false;
	}

	std::ifstream defaults_file(defaults_filename);
	if (! defaults_file.is_open())
	{
		std::cout << "Failed to open defaults file, " << defaults_filename << ".\n";
		defaults_file_ood = true;
	}
	else
	{
		defaults_file_ood = false;
	}
	
	parameter_set_complete = true;
	
	char config_line[max_string_size];
	char default_line[max_string_size];
	for (int line_num = 0; config_file.getline(config_line, max_string_size - 1) != nullptr; ++ line_num)
	{
		// read default value
		if (defaults_file.getline(default_line, max_string_size - 1) == nullptr)
		{
			std::cout << "ConfigManager: Unrecognised default parameter on line " << line_num << ".\n";
			std::cout << "ConfigManager: Program defaults will be used to remainder of values.\n";
			break;
		}

		// read name/value strings from config file
		char * value_string = strchr(config_line, '=');
		if (value_string == nullptr)
		{
			std::cout << "ConfigManager: Invalid line " << line_num << " of config file.\n";
			std::cout << "ConfigManager: Program defaults will be used for remainder of values.\n";
			break;
		}
		
		// determine "value_string=name_string".
		* (value_string ++) = '\0';
		char const * name_string = config_line;
		
		// Get the parameter in question, given its name.
		ConfigEntry * parameter = GetParameter(name_string);
		if (parameter == nullptr)
		{
			std::cout << "ConfigManager: unrecognised parameter " << name_string << " on line " << line_num << ".\n";
			std::cout << "ConfigManager: Program defaults will be used for remainder of values.\n";
			break;
		}
		
		if (parameter->Set(value_string, default_line))
		{
			defaults_file_ood = true;
		}
	}
	
	return true;
}

void ConfigManager::Save()
{
	parameter_set_complete = true;
	
	// open files
	std::ofstream config_file(config_filename);
	if (! config_file.is_open())
	{
		std::cout << "Failed to open config file, " << config_filename << ".\n";
	}
	
	std::ofstream defaults_file;
	if (defaults_file_ood)
	{
		defaults_file.open(defaults_filename);
		if (! defaults_file.is_open())
		{
			std::cout << "Failed to open config defaults file, " << defaults_filename << ".\n";
		}
	}
	
	for (ConfigEntry const * iterator = list_head; iterator != nullptr; iterator = iterator->next)
	{
		char config_string[max_string_size];
		char default_string[max_string_size];
		iterator->Get(config_string, default_string);
		config_file << iterator->name << "=" << config_string << '\n';
		defaults_file << default_string << '\n';
	}
}

void ConfigManager::AddParameter(ConfigEntry & parameter)
{
	Assert(! parameter_set_complete);
	Assert(parameter.next == nullptr);

	char const * name = parameter.name;
	Assert(name != nullptr);
	Assert(GetParameter(parameter.name) == nullptr);

	ConfigEntry * * ptr_iterator = & list_head;
	while (true)
	{
		ConfigEntry * next = * ptr_iterator;
		if (next == nullptr || strcmp(next->name, name) > 0)
		{
			break;
		}
		
		ptr_iterator = & (* ptr_iterator)->next;
	}
	
	parameter.next = * ptr_iterator;
	* ptr_iterator = & parameter;
}

ConfigEntry * ConfigManager::GetParameter(char const * name)
{
	for (ConfigEntry * iterator = list_head; iterator != nullptr; iterator = iterator->next)
	{
		if (strcmp(name, iterator->name) == 0)
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
