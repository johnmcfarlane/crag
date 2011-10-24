/*
 *  Regulator.h
 *  crag
 *
 *  Created by John on 10/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sys/App.h"


namespace form
{

	// The regulator class received performance-related samples from elsewhere in the simulation
	// and uses them to determine whether the load on the system should be increased or decreased.
	// TODO: Keep results as target num quaterna
	class Regulator
	{
		OBJECT_NO_COPY(Regulator);

	public:
		Regulator();
		
		void Reset();
		
		void SetNumQuaterna(int num_quaterne);
		void SampleFrameFitness(float fitness);
		void SampleMeshGenerationPeriod(sys::TimeType mgp);
		
		// returns a recommended load given the current load, and resets sample counters.
		int GetRecommendedNumQuaterna();
		
	private:
		int CalculateFrameRateDirectedTargetLoad(int current_load, sys::TimeType sim_time) const;
		int CalculateMeshGenerationDirectedTargetLoad(int current_load) const;
		static float CalculateFrameRateReactionCoefficient(sys::TimeType sim_time);
		
		sys::TimeType reset_time;
		
		int _num_quaterne;
		float frame_ratio_max;	// ~ (actual frame time / ideal frame time), i.e. greater is worse		
		float mesh_generation_period;	// again, greater is worse
	};

}
