//
//  Statistics.h
//  crag
//
//  Created by John McFarlane on 11/16/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Enumeration.h"


#if defined (CRAG_DEBUG)
#define GATHER_STATS
#endif


#if defined (GATHER_STATS)

namespace core
{
	
	////////////////////////////////////////////////////////////////////////////////
	// classes
	
	class StatInterface : public Enumeration<StatInterface>
	{
	public:
		StatInterface (char const * name, float verbosity);
		virtual ~StatInterface();
		
		float GetVerbosity () const;
		
		virtual void Write (std::ostream & out) const = 0;
		
		friend bool operator < (StatInterface const & lhs, StatInterface const & rhs);
		
	private:
		float _verbosity;
	};
	
	inline std::ostream & operator << (std::ostream & out, StatInterface const & stat)
	{
		stat.Write (out);
		return out;
	}
	
	template <typename VALUE_TYPE>
	class Stat : public StatInterface
	{
	public:
		typedef VALUE_TYPE value_type;
		
		Stat(char const * name, VALUE_TYPE init, float verbosity)
		: StatInterface(name, verbosity)
		, _value(init)
		{
		}
		
		virtual ~Stat()
		{
		}
		
		void Write (std::ostream & out) const
		{
			out << _value;
		}
		
		value_type _value;
	};
	
	class Statistics : public Enumeration <StatInterface>
	{
		OBJECT_NO_COPY(Statistics);

		typedef Enumeration <StatInterface> super;
	public:
	};
	
}

// TODO: Use TYPE() instead of 0 and decltype(DEFAULT) instead of TYPE
#define STAT(NAME, TYPE, VERBOSITY) ::core::Stat<TYPE> NAME##_stat (#NAME, 0, VERBOSITY)
#define STAT_DEFAULT(NAME, TYPE, VERBOSITY, DEFAULT) ::core::Stat<TYPE> NAME##_stat (#NAME, DEFAULT, VERBOSITY)
#define STAT_EXTERN(NAME, TYPE) extern ::core::Stat<TYPE> NAME##_stat
#define STAT_SET(NAME, VALUE) NAME##_stat._value = VALUE
#define STAT_INC(NAME, VALUE) (NAME##_stat._value += VALUE)

#else

#define STAT(NAME, TYPE, VERBOSITY) namespace NAME##_stat { }
#define STAT_DEFAULT(NAME, TYPE, VERBOSITY, DEFAULT) namespace NAME##_stat { }
#define STAT_EXTERN(NAME, TYPE) namespace NAME##_stat { }
#define STAT_SET(NAME, VALUE) DO_NOTHING
#define STAT_INC(NAME, VALUE) DO_NOTHING

#endif
