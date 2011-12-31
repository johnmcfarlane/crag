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

#include "core/app.h"
#include "core/ConfigEntry.h"


namespace 
{
	CONFIG_DEFINE (frame_rate_reaction_coefficient_base, float, 0.025f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost, float, 0.05f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost_half_life, float, 5.f);
	
	// TODO: Base it on full scene thread cycle.
	CONFIG_DEFINE (max_mesh_generation_period, float, 0.35f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
}


form::Regulator::Regulator()
: _enabled(true)
{
	Reset();
}

void form::Regulator::Reset()
{
	reset_time = app::GetTime();
	_num_quaterne = -1;
	frame_ratio_max = 0;
	mesh_generation_period = 0;
}

void form::Regulator::SetEnabled(bool enabled)
{
	_enabled = enabled;
}

void form::Regulator::SetNumQuaterna(int num_quaterne)
{
	_num_quaterne = num_quaterne;
}

// Take a sample of the frame ratio and apply it to the stored
// running maximum since the last adjustment.
void form::Regulator::SampleFrameFitness(float fitness)
{
	if (! _enabled)
	{
		return;
	}

	Assert(fitness >= 0);
	frame_ratio_max = std::max(frame_ratio_max, 1.f / fitness);
}

void form::Regulator::SampleMeshGenerationPeriod(Time mgp)
{
	if (! _enabled)
	{
		return;
	}

	Assert(mgp >= 0);
	mesh_generation_period = std::max(static_cast<float>(mgp), mesh_generation_period);
}

int form::Regulator::GetRecommendedNumQuaterna()
{
	if ((frame_ratio_max == 0 && mesh_generation_period == 0) || _num_quaterne == -1)
	{
		return _num_quaterne;
	}
	
	Time sim_time = app::GetTime() - reset_time;

	// Come up with two accounts of how many quaterna we should have.
	int frame_ratio_directed_target_load = CalculateFrameRateDirectedTargetLoad(_num_quaterne, sim_time);
	int mesh_generation_directed_target_load = CalculateMeshGenerationDirectedTargetLoad(_num_quaterne);

	// Pick the more conservative of the two numbers.
	int recommended_num_quaternia = std::min(mesh_generation_directed_target_load, frame_ratio_directed_target_load);
	
#if ! defined(NDEBUG) && 0
	if (recommended_num_quaternia < _num_quaterne)
	{
		int less_nodes = 1;
	}
	else if (recommended_num_quaternia > _num_quaterne)
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
	frame_ratio_max = 0;
	mesh_generation_period = 0;
	
	// return the result
	return recommended_num_quaternia;
}

// Taking into account the framerate ratio, come up with a quaterna count.
int form::Regulator::CalculateFrameRateDirectedTargetLoad(int current_load, Time sim_time) const
{
	// No samples to measure from?
	if (frame_ratio_max == 0)
	{
		// Don't recommend any change.
		return current_load;
	}
	
	// Attenuate/invert the frame_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_ratio_log = std::log(frame_ratio_max);
	float frame_ratio_exp = std::exp(frame_ratio_log * - CalculateFrameRateReactionCoefficient(sim_time));
	
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
	if (mesh_generation_period <= 0 || mesh_generation_period < max_mesh_generation_period)
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
float form::Regulator::CalculateFrameRateReactionCoefficient(Time sim_time)
{
	float boost_factor = pow(.5f, static_cast<float>(sim_time) / frame_rate_reaction_coefficient_boost_half_life);
	float boost_summand = frame_rate_reaction_coefficient_boost * boost_factor;
	float base_summand = frame_rate_reaction_coefficient_base;
	return boost_summand + base_summand;
}
