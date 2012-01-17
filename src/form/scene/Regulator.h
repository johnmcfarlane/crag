//
//  Regulator.h
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace form
{

	// The regulator class received performance-related samples from elsewhere in the simulation
	// and uses them to determine whether the load on the system should be increased or decreased.
	class Regulator
	{
		OBJECT_NO_COPY(Regulator);

	public:
		Regulator();
		
		void Reset();
		void SetEnabled(bool enabled);
		
		void SetNumQuaterna(int num_quaterne);
		void SampleFrameFitness(float fitness);
		void SampleMeshGenerationPeriod(Time mgp);
		
		// returns a recommended load given the current load, and resets sample counters.
		int GetRecommendedNumQuaterna();
		
	private:
		int CalculateFrameRateDirectedTargetLoad(int current_load, float frame_ratio, Time sim_time) const;
		int CalculateMeshGenerationDirectedTargetLoad(int current_load) const;
		static float CalculateFrameRateReactionCoefficient(Time sim_time);
		
		Time reset_time;

		bool _enabled;
		int _num_quaterne;
		int _frame_rate_num_quaterne;	// running frame rate-influenced recommendation
		float mesh_generation_period;	// again, greater is worse
	};

}
