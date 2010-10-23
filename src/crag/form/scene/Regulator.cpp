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
	CONFIG_DEFINE (frame_rate_reaction_coefficient_base, float, 0.01f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost, float, 0.05f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient_boost_half_life, float, 2.f);
	
	CONFIG_DEFINE (max_mesh_generation_period, float, 0.35f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
}


form::Regulator::Regulator()
: reset_time (sys::GetTime())
, frame_ratio(-1)
, mesh_generation_period(-1)
{
}

// Take a sample of the frame ratio and apply it to the stored
// running maximum since the last adjustment.
void form::Regulator::SampleFrameRatio(float fr)
{
	float clipped_frame_ratio = Max(fr, std::numeric_limits<float>::min());
	frame_ratio = Max(frame_ratio, clipped_frame_ratio);
}

void form::Regulator::SampleMeshGenerationPeriod(sys::TimeType mgp)
{
	Assert(mgp >= 0);
	mesh_generation_period = Max(static_cast<float>(mgp), mesh_generation_period);
}

int form::Regulator::GetAdjustedLoad(int current_load)
{
	// TODO: Do we want a similar rule for mesh generation?
	if (frame_ratio < 0)
	{
		return current_load;
	}
	
	// Come up with two accounts of how many quaterna we should have.
	int frame_ratio_directed_target_load = CalculateFrameRateDirectedTargetLoad(current_load);
	int mesh_generation_directed_target_load = CalculateMeshGenerationDirectedTargetLoad(current_load);

	// Pick the more conservative of the two numbers.
	int adjusted_load = Min(mesh_generation_directed_target_load, frame_ratio_directed_target_load);
	
	// Reset the parameters used to come to a decision so
	// we don't just keep acting on them over and over. 
	mesh_generation_period = -1;
	frame_ratio = -1;

	// return the result
	return adjusted_load;
}

// Taking into account the framerate ratio, come up with a quaterna count.
int form::Regulator::CalculateFrameRateDirectedTargetLoad(int current_load) const
{
	sys::TimeType t = sys::GetTime() - reset_time;
	
	// Attenuate/invert the frame_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_ratio_log = Log(frame_ratio);
	float frame_ratio_exp = Exp(frame_ratio_log * - CalculateFrameRateReactionCoefficient(t));
	
	int frame_ratio_directed_target_num_quaterna = static_cast<int>(static_cast<float>(current_load) * frame_ratio_exp) + 1;
	
	// Ensure the value is suitably clamped. 
	Assert(frame_ratio_directed_target_num_quaterna > 0);
	
	return frame_ratio_directed_target_num_quaterna;
}

// Taking into account how long it took to generate the last mesh for the renderer, come up with a quaterna count.
int form::Regulator::CalculateMeshGenerationDirectedTargetLoad(int current_load) const
{
	// If we don't have a current reading.
	if (mesh_generation_period < 0)
	{
	}
	
	// We're under budget so everything's ok.
	if (mesh_generation_period < max_mesh_generation_period)
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
