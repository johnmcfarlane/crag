//
//  ConfigEntry.h
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


#define ENABLE_CONFIG

#if defined(ENABLE_CONFIG)

#include "Enumeration.h"


namespace core
{
	
	class ConfigEntry : public Enumeration<ConfigEntry>
	{
	public:
		ConfigEntry(char const * name) 
		: Enumeration<ConfigEntry>(name) 
		{ 
		}
		
		virtual ~ConfigEntry()
		{
		}
		
		virtual void Get(char * config_string, char * default_string) const = 0;

		// Returns true iff the default has changed.
		virtual bool Set(char const * config_string, char const * default_string) = 0;
	};


	template<typename S> class Config : protected ConfigEntry
	{
	public:
		Config(S & var, char const * init_name) 
		: ConfigEntry(init_name)
		, variable(var)
		, default_value(var)
		{ 
		}
		
		virtual void Get(char * config_string, char * default_string) const
		{
			ValueToString(config_string, variable);
			ValueToString(default_string, default_value);
		}
		
		virtual bool Set(char const * config_string, char const * default_string) 
		{
			S stored_default_value;
			StringToValue(stored_default_value, default_string);
			if (stored_default_value == default_value)
			{
				StringToValue(variable, config_string);
				return false;
			}
			
			Assert(default_value == variable);
			return true;
		}
		
	private:
		static int ValueToString(char * string, S const & value);
		static int StringToValue(S & value, char const * string);
		
		S & variable;
		S const default_value;
	};


	template<typename S> class ConfigAngle : protected Config<S>
	{
	public:
		ConfigAngle(S & init_var, char const * init_name) 
		: Config<S>(init_var, init_name)
		{ 
		}
		
		static int ValueToString(char * string, S const & value);
		static int StringToValue(S & value, char const * string);
	};

}


#define CONFIG_DEFINE(name, type, default) type name = default; core::Config<type> name##config (name, #name)
#define CONFIG_DEFINE_ANGLE(name, type, default) type name = DegToRad(default); core::ConfigAngle<type> name##config (name, #name)

#else

#define CONFIG_DEFINE(name, type, default) type name = default
#define CONFIG_DEFINE_ANGLE(name, type, default) type name = DegToRad(default)

#endif

#define CONFIG_DECLARE(name, type) extern type name

