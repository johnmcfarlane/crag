/*
 *  Profile.cpp
 *  Crag
 *
 *  Created by john on 6/08/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
*
*/

#include "pch.h"

#if defined(PROFILE)

#include "profile.h"


////////////////////////////////////////////////////////////////////////////////
// profile::Meter member definitions

profile::Meter::Meter(Scalar const & init_change_coefficient)
: store(-1)
, change_coefficient(init_change_coefficient)
{
}

profile::Meter::operator profile::Scalar () const
{
	return store;
}

void profile::Meter::Submit (Scalar sample)
{
	if (! IsNaN(sample) && ! IsInf(sample))
	{
		Assert(sample != std::numeric_limits<Scalar>::infinity());
		Assert(change_coefficient > 0 && change_coefficient < 1);
		if (store >= 0)
		{
			store = store * (static_cast<Scalar>(1) - change_coefficient) + sample * change_coefficient;
		}
		else
		{
			store = sample;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// profile::Timer member definitions

profile::Timer::Timer()
: start_time(sys::GetTime())
{
}

profile::Timer::operator profile::Scalar () const
{
	sys::TimeType now = sys::GetTime();
	sys::TimeType duration = now - start_time;
	return profile::Scalar(duration);
}

#endif