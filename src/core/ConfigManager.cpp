//
//  ConfigManager.cpp
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ConfigManager.h"
#include "ConfigEntry.h"

#if defined(ENABLE_CONFIG)

#include "core/debug.h"


#if ! defined(NDEBUG) || ! defined(PROFILE)
#define ENABLE_CONFIG_LOAD_SAVE
#endif


namespace 
{
	int const max_string_size = 1024;

#if defined(ENABLE_CONFIG_LOAD_SAVE)
	bool parameter_set_complete = false;	
	bool defaults_file_ood = false;

	char const * config_filename = "crag.cfg";
	char const * defaults_filename = "default.cfg";
#endif
}


//////////////////////////////////////////////////////////////////////
// Config function definitions


using core::ConfigManager;


ConfigManager::ConfigManager()
{
	if (! Load())
	{
		// Make sure we always have a config file handy.
		Save();
	}
}

ConfigManager::~ConfigManager()
{
	Save();
}

bool ConfigManager::Load()
{
#if defined(ENABLE_CONFIG_LOAD_SAVE)
	// open files
	std::ifstream config_file(config_filename);
	if (! config_file.is_open())
	{
		ERROR_MESSAGE("Failed to open config file, %s.", config_filename);
		return false;
	}

	std::ifstream defaults_file(defaults_filename);
	if (! defaults_file.is_open())
	{
		ERROR_MESSAGE("Failed to open defaults file, %s.", defaults_filename);
		defaults_file_ood = true;
	}
	else
	{
		defaults_file_ood = false;
	}
	
	parameter_set_complete = true;
	
	char config_line[max_string_size];
	char default_line[max_string_size];
	for (int line_num = 0; ! config_file.eof(); ++ line_num)
	{
		// read default value
		if (defaults_file.eof())
		{
			ERROR_MESSAGE("ConfigManager: Unrecognised default parameter on line %d.", line_num);
			ERROR_MESSAGE("ConfigManager: Program defaults will be used for remainder of values.");
			break;
		}
		
		config_file.getline(config_line, max_string_size - 1);
		defaults_file.getline(default_line, max_string_size - 1);
		
		if (config_line[0] == '\0')
		{
			continue;
		}

		// read name/value strings from config file
		char * value_string = strchr(config_line, '=');
		if (value_string == nullptr)
		{
			ERROR_MESSAGE("ConfigManager: Invalid line %d of config file.", line_num);
			ERROR_MESSAGE("ConfigManager: Program defaults will be used for remainder of values.");
			break;
		}
		
		// determine "value_string=name_string".
		* (value_string ++) = '\0';
		char const * name_string = config_line;
		
		// Get the parameter in question, given its name.
		ConfigEntry * parameter = ConfigEntry::find(name_string);
		if (parameter == nullptr)
		{
			ERROR_MESSAGE("ConfigManager: unrecognised parameter \"%s\" on line %d.", name_string, line_num);
			ERROR_MESSAGE("ConfigManager: Program defaults will be used for remainder of values.");
			break;
		}
		
		if (parameter->Set(value_string, default_line))
		{
			defaults_file_ood = true;
		}
	}
#endif
	
	return true;
}

void ConfigManager::Save()
{
#if defined(ENABLE_CONFIG_LOAD_SAVE)
	parameter_set_complete = true;
	
	// open files
	std::ofstream config_file(config_filename);
	if (! config_file.is_open())
	{
		ERROR_MESSAGE("Failed to open config file, %s.", config_filename);
	}
	
	std::ofstream defaults_file;
	if (defaults_file_ood)
	{
		defaults_file.open(defaults_filename);
		if (! defaults_file.is_open())
		{
			ERROR_MESSAGE("Failed to open config defaults file, %s.", defaults_filename);
		}
	}
	
	for (ConfigEntry::iterator i = ConfigEntry::begin(); i != ConfigEntry::end(); ++ i)
	{
		char config_string[max_string_size];
		char default_string[max_string_size];
		ConfigEntry & config_entry = * i;
		config_entry.Get(config_string, default_string);
		config_file << config_entry.GetName() << "=" << config_string << '\n';
		defaults_file << default_string << '\n';
	}
#endif
}

bool ConfigManager::ParseCommandLine(int argc, char * const * argv)
{
	char current_value[max_string_size];
	char default_value[max_string_size];
	
	auto get_value = [] (char * const argument) -> char const *
	{
		auto found = strchr(argument, '=');
		if (found == nullptr)
		{
			return "1";
		}

		* found = '\0';
		return found + 1;
	};

	for (; argc > 0; ++ argv, -- argc)
	{
		auto name_string = * argv;
		auto value_string = get_value(* argv);
		
		// Get the parameter in question, given its name.
		auto parameter = ConfigEntry::find(name_string);
		if (parameter == nullptr)
		{
			ERROR_MESSAGE("ConfigManager: unrecognised parameter \"%s\" in command %s.", name_string, * argv);
			return false;
		}
		
		parameter->Get(current_value, default_value);
		parameter->Set(value_string, default_value);
	}
	
	return true;
}

#else

bool ConfigManager::Load()
{
	return true;
}

void ConfigManager::Save()
{
}

bool ConfigManager::ParseCommandLine(int, char * const *)
{
	return true
}

#endif	// ENABLE_CONFIG
