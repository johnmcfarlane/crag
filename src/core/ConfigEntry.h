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

namespace crag {
namespace core {
namespace config
{
	class Entry : public ::core::Enumeration<Entry>
	{
	public:
		Entry(char const * name)
			: ::core::Enumeration<Entry>(name)
		{
		}

		virtual ~Entry()
		{
		}

		virtual void Get(char * config_string, char * default_string) const = 0;

		// Returns true iff the default has changed.
		virtual bool Set(char const * config_string, char const * default_string) = 0;
	};

	template <typename TYPE>
	class GenericEntry : Entry
	{
	public:
		using value_type = TYPE;

		template <typename IN_TYPE>
		GenericEntry(IN_TYPE & var, char const * init_name)
		: Entry(init_name)
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
			value_type stored_default_value;
			StringToValue(stored_default_value, default_string);
			if (stored_default_value == default_value)
			{
				StringToValue(variable, config_string);
				return false;
			}
			
			ASSERT(default_value == variable);
			return true;
		}
		
	private:
		static int ValueToString(char * string, value_type const & value);
		static int StringToValue(value_type & value, char const * string);
		
		value_type & variable;
		value_type const default_value;
	};

	template<typename TYPE> 
	class GenericEntryAngle : protected GenericEntry<TYPE>
	{
	public:
		GenericEntryAngle(value_type & init_var, char const * init_name)
		: GenericEntry<value_type>(init_var, init_name)
		{ 
		}
		
		static int ValueToString(char * string, value_type const & value);
		static int StringToValue(value_type & value, char const * string);
	};
}}}



#define CONFIG_DEFINE(name, default_value) \
	decltype(default_value) name = default_value; \
	::crag::core::config::GenericEntry<decltype(default_value)> name##config (name, #name)

#define CONFIG_DEFINE_ANGLE(name, default_value) \
	decltype(default_value) name = DegToRad(default_value); \
	::crag::core::config::GenericEntryAngle<decltype(default_value)> name##config (name, #name)

#else

#define CONFIG_DEFINE(name, type, default) type name = default
#define CONFIG_DEFINE_ANGLE(name, type, default) type name = DegToRad(default)

#endif

#define CONFIG_DECLARE(name, type) extern type name
#define CONFIG_DECLARE_ANGLE(name, type) extern type name
