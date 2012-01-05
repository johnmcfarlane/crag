//
//  FrequencyEstimator.cpp
//  crag
//
//  Created by John on 10/20/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FrequencyEstimator.h"

#include "core/debug.h"


///////////////////////////////////////////////////////////////
// Estimator

Estimator::Estimator(float init_dampening_coefficient)
: dampening_coefficient(init_dampening_coefficient)
, last_value(0)
, estimated_average(0)
, has_samples(false)
{
	Assert(dampening_coefficient >= 0);	// At zero, this simply measures the frequency of the last tick.
	Assert(dampening_coefficient < 1);	// At one, this never takes new ticks into account at all.
}

void Estimator::AddSample(float value)
{
	last_value = value;
	if (has_samples) {
		estimated_average = 
			(estimated_average * dampening_coefficient)		// The old component.
			+	(last_value * (1.f - dampening_coefficient));	// The new component.
	}
	else {
		estimated_average = value;
		has_samples = true;
	}
}


///////////////////////////////////////////////////////////////
// FrequencyEstimator

FrequencyEstimator::FrequencyEstimator(float init_dampening_coefficient)
: Estimator(init_dampening_coefficient)
, last_tick(-1)
{
}

// The input.
void FrequencyEstimator::OnTick(float time)
{
	Assert(time >= 0);
	//Assert(time >= last_tick);
	
	if (last_tick > 0)
	{
		AddSample(time - last_tick);
	}
	
	last_tick = time;
}

