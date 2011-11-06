/*
 *  ProfileEntry.h
 *  Crag
 *
 *  Created by john on 6/08/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sys/App.h"


#if defined(PROFILE)

namespace profile
{
	// Determines the accuracy of the profile results.
	typedef float Scalar;
	
	
	class Meter
	{
	public:
		Meter(Scalar const & init_change_coefficient);
		
		operator Scalar () const;
		
		void Submit (Scalar sample);
		
	private:
		
		Scalar store;
		Scalar change_coefficient;
	};


	class Timer
	{
	public:
		Timer();
		operator Scalar () const;
		
	private:
		sys::Time start_time;
	};
	
}

#define PROFILE_DEFINE(NAME, CHANGE_COEFFICIENT) profile::Meter NAME(CHANGE_COEFFICIENT)
#define PROFILE_SAMPLE(NAME, SAMPLE) NAME.Submit(static_cast<profile::Scalar>(SAMPLE))
#define PROFILE_TIMER_BEGIN(NAME) profile::Timer PROFILE_TIMER_##NAME
#define PROFILE_TIMER_READ(NAME) (static_cast<profile::Scalar>(PROFILE_TIMER_##NAME))
#define PROFILE_RESULT(NAME) NAME

#else

#define PROFILE_DEFINE(NAME, CHANGE_COEFFICIENT) 
#define PROFILE_SAMPLE(NAME, SAMPLE) DO_NOTHING
#define PROFILE_TIMER_BEGIN(NAME) DO_NOTHING
#define PROFILE_TIMER_READ(NAME) (-1)
#define PROFILE_RESULT(NAME) (-1)

#endif
