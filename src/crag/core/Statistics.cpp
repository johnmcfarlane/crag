/*
 *  Statistics.cpp
 *  crag
 *
 *  Created by John McFarlane on 11/16/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Statistics.h"


#if defined (GATHER_STATS)

using core::StatInterface;

StatInterface::StatInterface(char const * name, float verbosity)
: Enumeration<StatInterface>(name)
, _verbosity(verbosity)
{
}

float StatInterface::GetVerbosity() const
{
	return _verbosity;
}

// TODO: Reinstate. (Oh, and what does it do??)
//bool core::operator < (StatInterface const & lhs, StatInterface const & rhs)
//{
//	float verbosity_difference = lhs.GetVerbosity() - rhs.GetVerbosity();
//	if (verbosity_difference < 0)
//	{
//		return true;
//	}
//	else if (verbosity_difference > 0)
//	{
//		return false;
//	}
//	else
//	{
//		return Enumeration<StatInterface>::sort_function(lhs, rhs);
//	}
//}

#endif
