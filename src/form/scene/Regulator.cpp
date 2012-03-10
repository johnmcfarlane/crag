//
//  Regulator.cpp
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Regulator.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


namespace 
{
	CONFIG_DEFINE (frame_rate_reaction_coefficient_base, float, 0.025f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost, float, 0.225f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost_half_life, float, 10.f);
	
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
	_frame_rate_num_quaterne = -1;
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
void form::Regulator::SampleFrameDuration(float frame_duration_ratio)
{
	if (! _enabled || _num_quaterne == -1 || frame_duration_ratio == 0)
	{
		return;
	}

	ASSERT(frame_duration_ratio >= 0);
	ASSERT(! IsInf(frame_duration_ratio));
	
	Time sim_time = app::GetTime() - reset_time;
	int recommended_this_frame = CalculateFrameRateDirectedTargetLoad(_num_quaterne, frame_duration_ratio, sim_time);
	if (_frame_rate_num_quaterne == -1 || recommended_this_frame < _frame_rate_num_quaterne)
	{
		_frame_rate_num_quaterne = recommended_this_frame;
	}
}

void form::Regulator::SampleMeshGenerationPeriod(Time mgp)
{
	if (! _enabled)
	{
		return;
	}

	ASSERT(mgp >= 0);
	mesh_generation_period = std::max(static_cast<float>(mgp), mesh_generation_period);
}

int form::Regulator::GetRecommendedNumQuaterna()
{
	if (_num_quaterne == -1)
	{
		return _num_quaterne;
	}
	
	// The recommended num comes from two sources: frame ratio,
	int recommended_num_quaternia = _frame_rate_num_quaterne;
	
	// and mesh generation time.
	int mesh_generation_directed_target_load = CalculateMeshGenerationDirectedTargetLoad(_num_quaterne);
	if (mesh_generation_directed_target_load != -1)
	{
		if (recommended_num_quaternia == -1 || mesh_generation_directed_target_load < recommended_num_quaternia)
		{
			recommended_num_quaternia = mesh_generation_directed_target_load;
		}
	}
	
	// If neither source had an 'opinion', don't change anything.
	if (recommended_num_quaternia < 0)
	{
		recommended_num_quaternia = _num_quaterne;
	}
	
#if ! defined(NDEBUG) && 0
	static int fewer = 0, more = 0, no_change = 0;
	if (recommended_num_quaternia < _num_quaterne)
	{
		++ fewer;
	}
	else if (recommended_num_quaternia > _num_quaterne)
	{
		++ more;
	}
	else
	{
		++ no_change;
	}
	if (fewer + more + no_change == 1000)
	{
		std::cout << '[' << fewer << ' ' << no_change << ' ' << more << ']' << std::endl;
		fewer = more = no_change = 0;
	}
#endif
	
	// Reset the parameters used to come to a decision so
	// we don't just keep acting on them over and over. 
	_frame_rate_num_quaterne = -1;
	mesh_generation_period = 0;
	
	// return the result
	return recommended_num_quaternia;
}

// Taking into account the framerate ratio, come up with a quaterna count.
int form::Regulator::CalculateFrameRateDirectedTargetLoad(int current_load, float frame_rate_ratio, Time sim_time) const
{
	// Attenuate/invert the frame_rate_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_rate_ratio_log = std::log(frame_rate_ratio);
	float frame_rate_ratio_exp = std::exp(frame_rate_ratio_log * - CalculateFrameRateReactionCoefficient(sim_time));
	
	float exact_target_load = static_cast<float>(current_load) * frame_rate_ratio_exp;
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
	ASSERT(target_load >= 0);
	
	return target_load;
}

// Taking into account how long it took to generate the last mesh for the renderer, come up with a quaterna count.
int form::Regulator::CalculateMeshGenerationDirectedTargetLoad(int current_load) const
{
	// If we don't have a current reading or we're under budget everything's ok.
	if (mesh_generation_period <= 0 || mesh_generation_period < max_mesh_generation_period)
	{
		// As far as this function's concerned, increase the count to anything you like (within reason).
		return -1;
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
