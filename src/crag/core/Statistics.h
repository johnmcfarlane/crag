/*
 *  Statistics.h
 *  crag
 *
 *  Created by John McFarlane on 11/16/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Enumeration.h"


#if defined (PROFILE)
#define GATHER_STATS
#endif


#if defined (GATHER_STATS)

namespace core
{

	////////////////////////////////////////////////////////////////////////////////
	// forward-declaration
	
	class StatInterface;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// classes
	
	class Statistics : public Enumeration <StatInterface>
	{
	public:
	};

	class StatInterface : public Statistics::node
	{
		typedef Statistics::node super;
		
	public:
		StatInterface (char const * name, float verbosity);
		
		float GetVerbosity () const;
		
		virtual void Write (std::ostream & out) const = 0;
		
		friend int operator - (StatInterface const & lhs, StatInterface const & rhs);

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
		
		Stat(char const * name, float verbosity)
		: StatInterface(name, verbosity)
		{
		}

		void Write (std::ostream & out) const
		{
			out << _value;
		}
		
		value_type _value;
	};
	
}


#define STAT(NAME, TYPE, VERBOSITY) core::Stat<TYPE> NAME##_stat (#NAME, VERBOSITY)
#define STAT_SET(NAME, VALUE) NAME##_stat._value = VALUE

#else

#define STAT(NAME, TYPE, VERBOSITY) extern int NAME##_stat
#define STAT_SET(NAME, VALUE) DO_NOTHING

#endif
