//
//  RegulatorScript.h
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009 - 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "script/ScriptBase.h"


namespace form
{

	// The regulator class received performance-related samples from elsewhere in the simulation
	// and uses them to determine whether the load on the system should be increased or decreased.
	class RegulatorScript : public script::ScriptBase
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		enum UnitType
		{
			frame_rate,
			mesh_generation,
			num_units
		};
	public:
		class Unit;
		
		class QuaterneCount
		{
		public:
			////////////////////////////////////////////////////////////////////////////////
			// functions
			QuaterneCount();
			QuaterneCount(int num_quaterne);
			
#if defined(VERIFY)
			void Verify() const;
#endif
			QuaterneCount & operator=(QuaterneCount rhs);
			
			bool operator < (QuaterneCount const & rhs) const;
			
			QuaterneCount & operator ++ ();
			QuaterneCount & operator -- ();

			int GetNumber() const;
			
			static QuaterneCount max();
			static QuaterneCount invalid();
			
		private:
			////////////////////////////////////////////////////////////////////////////////
			// variables
			int _num;
		};

		////////////////////////////////////////////////////////////////////////////////
		// functions
		RegulatorScript();
		
		// script entry point
		virtual void operator() (script::FiberInterface & fiber) override;
		
		// inputs - typically called from form/gfx threads
		void SetNumQuaterne(int const & num_quaterne);
		void SampleFrameDuration(float const & frame_duration_ratio);
		void SampleMeshGenerationPeriod(Time const & mesh_generation_period);
		
	private:
		QuaterneCount GetRecommendedNumQuaterna() const;
		
		void Reset();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Unit * _units[num_units];
		QuaterneCount _current_num_quaterne;
	};
}
