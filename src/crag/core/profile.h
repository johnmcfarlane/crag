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

#include "sys/time.h"


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
		Timer(Meter & m);
		~Timer();
		
	private:
		Meter & meter;
		sys::TimeType start_time;
	};
	
}

#define PROFILE_DEFINE(NAME, CHANGE_COEFFICIENT) profile::Meter NAME(CHANGE_COEFFICIENT)
#define PROFILE_SAMPLE(NAME, SAMPLE) NAME.Submit(SAMPLE)
#define PROFILE_TIMER(NAME) profile::Timer PROFILE_TIMER_##NAME(NAME)
#define PROFILE_RESULT(NAME) NAME

#else

#define PROFILE_DEFINE(NAME, CHANGE_COEFFICIENT) 
#define PROFILE_SAMPLE(NAME, SAMPLE) DO_NOTHING
#define PROFILE_TIMER(NAME) DO_NOTHING
#define PROFILE_RESULT(NAME) (-1)

#endif
