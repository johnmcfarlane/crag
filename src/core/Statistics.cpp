//
//  Statistics.cpp
//  crag
//
//  Created by John McFarlane on 11/16/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Statistics.h"


#if defined (GATHER_STATS)

using core::StatInterface;

StatInterface::StatInterface(char const * name, float verbosity)
: Enumeration<StatInterface>(name)
, _verbosity(verbosity)
{
}

StatInterface::~StatInterface()
{
}

float StatInterface::GetVerbosity() const
{
	return _verbosity;
}

#endif
