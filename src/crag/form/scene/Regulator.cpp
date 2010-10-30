/*
 *  Regulator.cpp
 *  crag
 *
 *  Created by John on 10/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Regulator.h"

#include "core/ConfigEntry.h"


namespace 
{
	CONFIG_DEFINE (frame_rate_reaction_coefficient_base, float, 0.015f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost, float, 0.05f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost_half_life, float, 3.f);
	
	// TODO: Base it on full scene thread cycle.
	CONFIG_DEFINE (max_mesh_generation_period, float, 1.35f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
}


form::Regulator::Regulator()
: reset_time (sys::GetTime())
, frame_ratio_sum(0)
, frame_ratio_count(0)
, mesh_generation_period(-1)
{
}

// Take a sample of the frame ratio and apply it to the stored
// running maximum since the last adjustment.
void form::Regulator::SampleFrameRatio(float fr)
{
	Assert(fr >= 0);

	//	frame_ratio_sum += fr;
	//	++ frame_ratio_count;
	frame_ratio_sum = Max(frame_ratio_sum, fr);
	frame_ratio_count = 1;
}

void form::Regulator::SampleMeshGenerationPeriod(sys::TimeType mgp)
{
	Assert(mgp >= 0);
	mesh_generation_period = Max(static_cast<float>(mgp), mesh_generation_period);
}

int form::Regulator::GetAdjustedLoad(int current_load)
{
	// Come up with two accounts of how many quaterna we should have.
	int frame_ratio_directed_target_load = CalculateFrameRateDirectedTargetLoad(current_load);
	int mesh_generation_directed_target_load = CalculateMeshGenerationDirectedTargetLoad(current_load);

	// Pick the more conservative of the two numbers.
	int adjusted_load = Min(mesh_generation_directed_target_load, frame_ratio_directed_target_load);
	
#if ! defined(NDEBUG)
	if (adjusted_load < current_load)
	{
		int less_nodes = 1;
	}
	else if (adjusted_load > current_load)
	{
		int more_nodes = 1;
	}
	else
	{
		int no_change = 1;
	}
#endif

	// Reset the parameters used to come to a decision so
	// we don't just keep acting on them over and over. 
	mesh_generation_period = -1;
	frame_ratio_sum = 0;
	frame_ratio_count = 0;
	
	// return the result
	return adjusted_load;
}

// Taking into account the framerate ratio, come up with a quaterna count.
int form::Regulator::CalculateFrameRateDirectedTargetLoad(int current_load) const
{
	// No samples to measure from?
	if (frame_ratio_count == 0)
	{
		// Don't recommend any change.
		return current_load;
	}
	
	sys::TimeType t = sys::GetTime() - reset_time;
	float frame_ratio_avg = frame_ratio_sum / frame_ratio_count;
	if (frame_ratio_avg <= 0)
	{
		Assert(frame_ratio_avg == 0);
		return current_load + 1;
	}
	
	// Attenuate/invert the frame_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_ratio_log = Log(frame_ratio_avg);
	float frame_ratio_exp = Exp(frame_ratio_log * - CalculateFrameRateReactionCoefficient(t));
	
	float exact_target_load = static_cast<float>(current_load) * frame_ratio_exp;
	int target_load = static_cast<int>(exact_target_load);
	
	if (target_load == current_load)
	{
		if (exact_target_load >= 0)
		{
			++ target_load;
		}
		else 
		{
			-- target_load;
		}
	}
	
	// Ensure the value is suitably clamped. 
	Assert(target_load >= 0);
	
	return target_load;
}

// Taking into account how long it took to generate the last mesh for the renderer, come up with a quaterna count.
int form::Regulator::CalculateMeshGenerationDirectedTargetLoad(int current_load) const
{
	// If we don't have a current reading or we're under budget everything's ok.
	if (mesh_generation_period < 0 || mesh_generation_period < max_mesh_generation_period)
	{
		// As far as this function's concerned, increase the count to anything you like (within reason).
		return std::numeric_limits<int>::max();
	}
	
	// Reset this so there isn't a continuous decrease in node-count before the next recording.
	int mesh_generation_directed_target_load = static_cast<int>(current_load * max_mesh_generation_reaction_coefficient) - 1;
	if (mesh_generation_directed_target_load < 0)
	{
		mesh_generation_directed_target_load = 0;
	}
	
	return mesh_generation_directed_target_load;
}

// When the simulation first fires up, we want the number of quaterna to quickly adapt to the
// abilities of the hardware, which means a high frame_rate_reaction_coefficient.
// However, a highly reactionary regulatory system can cause hunting. 
// Plus large changes in the number of quaterna causes additional slowdown of its own 
// which feeds back into the system in a way which causes yet more hunting-type behavior.
// Thus the reaction coefficient is given a boost which decays over time. 
float form::Regulator::CalculateFrameRateReactionCoefficient(sys::TimeType t)
{
	float boost_factor = Power(.5f, static_cast<float>(t) / frame_rate_reaction_coefficient_boost_half_life);
	float boost_summand = frame_rate_reaction_coefficient_boost * boost_factor;
	float base_summand = frame_rate_reaction_coefficient_base;
	return boost_summand + base_summand;
}
