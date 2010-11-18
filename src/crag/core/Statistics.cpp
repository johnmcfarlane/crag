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
: super(name)
, _verbosity(verbosity)
{
}

float StatInterface::GetVerbosity() const
{
	return _verbosity;
}

bool core::operator < (StatInterface const & lhs, StatInterface const & rhs)
{
	float verbosity_difference = lhs.GetVerbosity() - rhs.GetVerbosity();
	if (verbosity_difference < 0)
	{
		return true;
	}
	else if (verbosity_difference > 0)
	{
		return false;
	}
	else
	{
		core::Statistics::node const & lhs_node = lhs;
		core::Statistics::node const & rhs_node = rhs;
		return lhs_node < rhs_node;
	}
}

#endif
