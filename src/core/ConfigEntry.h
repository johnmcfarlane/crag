//
//  ConfigEntry.h
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if defined(CRAG_PC)
#define ENABLE_CONFIG
#endif

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

		virtual bool Get(char * string, std::size_t max_string_size) const = 0;

		// Returns true iff the default has changed.
		virtual bool Set(char const * string) = 0;
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
		{
		}
		
		virtual bool Get(char * string, std::size_t max_string_size) const
		{
			return ValueToString(variable, string, max_string_size);
		}
		
		virtual bool Set(char const * string)
		{
			return StringToValue(variable, string);
		}
		
	private:
		static bool ValueToString(value_type const & value, char * string, std::size_t max_string_size);
		static bool StringToValue(value_type & value, char const * string);
		
		value_type & variable;
	};

	template<typename TYPE> 
	class GenericEntryAngle : protected GenericEntry<TYPE>
	{
		using _Base = GenericEntry<TYPE>;
	public:
		using value_type = typename _Base::value_type;

		GenericEntryAngle(value_type & init_var, char const * init_name)
		: GenericEntry<value_type>(init_var, init_name)
		{ 
		}
		
		static bool ValueToString(value_type const & value, char * string, std::size_t max_string_size);
		static bool StringToValue(value_type & value, char const * string);
	};

	// contains the complete list of configuration settings; returns false on fatal error
	bool Init(int argc, char * const * argv);
}}}



#define CONFIG_DEFINE(name, default_value) \
	decltype(default_value) name = default_value; \
	::crag::core::config::GenericEntry<decltype(default_value)> name##config (name, #name)

#define CONFIG_DEFINE_ANGLE(name, default_value) \
	decltype(default_value) name = DegToRad(default_value); \
	::crag::core::config::GenericEntryAngle<decltype(default_value)> name##config (name, #name)

#else

#define CONFIG_DEFINE(name, default_value) \
	decltype(default_value) name = default_value;

#define CONFIG_DEFINE_ANGLE(name, default_value) \
	decltype(default_value) name = DegToRad(default_value);

#endif

#define CONFIG_DECLARE(name, type) extern type name
#define CONFIG_DECLARE_ANGLE(name, type) extern type name
