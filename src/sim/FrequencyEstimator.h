//
//  FrequencyEstimator.h
//  crag
//
//  Created by John on 10/20/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/debug.h"


class Estimator
{
public:
	Estimator(float init_dampening_coefficient);
	
	// Has AddSample been called yet?
	bool HasSamples() const {
		return has_samples;
	}

	void AddSample(float value);
	
	// Undefined if HasSamples is false.
	float GetEstimatedAverage() const {
		Assert(HasSamples());
		return estimated_average;
	}
	
	// Undefined if HasSamples is false.
	float GetLastValue() const {
		Assert(HasSamples());
		return last_value;
	}
	
	float GetDampeningCoefficient() const {
		return dampening_coefficient;
	}
	
private:
	float dampening_coefficient;	// Dictates how far back ticks are considered. [0, 1)
	float last_value;
	float estimated_average;
	bool has_samples;
};


class FrequencyEstimator : private Estimator
{
public:
	FrequencyEstimator(float init_dampening_coefficient);
	
	// The input.
	void OnTick(float time);
	
	// Has AddSample been called yet?
	bool HasSamples() const {
		return Estimator::HasSamples();
	}

	// The estimates.
	float GetFrequency() const
	{
		if (HasSamples()) {
			float estimated_period = GetEstimatedAverage();
			return 1.f / estimated_period;
		}
		else {
			return 0;
		}
	}
	
	float GetEstimatedPeriod() const
	{
		float estimated_period = GetEstimatedAverage();
		return (estimated_period > 0) ? estimated_period : 0.f;
	}
	
	// Misc accessors.
	float GetLastTick() const
	{
		return last_tick;
	}
	
	float GetLastPeriod() const
	{
		return GetLastValue();
	}
	
	// Negative if OnTick has not been called yet (and no estimate can be made).
	/*float GetEstimatedPeriod() const
	{
		return estimated_period;
	}*/
	
private:
	
	float last_tick;				// The last time a tick occurred.
};

