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
	Assert(sample >= 0);
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


////////////////////////////////////////////////////////////////////////////////
// profile::Timer member definitions

profile::Timer::Timer(Meter & m)
: meter(m)
, start_time(sys::GetTime())
{
}

profile::Timer::~Timer()
{
	sys::TimeType end_time = sys::GetTime();
	sys::TimeType duration = end_time - start_time;
	meter.Submit(static_cast<Scalar>(duration));
}

#endif
