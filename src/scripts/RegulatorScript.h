//
//  RegulatorScript.h
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009 - 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "applet/Engine.h"

#include "gfx/Engine.h"
#include "gfx/Messages.h"

#include "ipc/Listener.h"

namespace applet
{
	class AppletInterface;
}

namespace script
{
	// The regulator class received performance-related samples from elsewhere in the simulation
	// and uses them to determine whether the load on the system should be increased or decreased.
	class RegulatorScript 
		: ipc::Listener<applet::Engine, gfx::NumQuaterneSetMessage>
		, ipc::Listener<applet::Engine, gfx::FrameDurationSampledMessage>
		, ipc::Listener<applet::Engine, gfx::MeshGenerationPeriodSampledMessage>
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
		~RegulatorScript();
		
		// script entry point
		void operator() (applet::AppletInterface & applet_interface);
		
		// Listener callbacks
		void operator() (gfx::NumQuaterneSetMessage const & message) final;
		void operator() (gfx::FrameDurationSampledMessage const & message) final;
		void operator() (gfx::MeshGenerationPeriodSampledMessage const & message) final;
		
	private:
		QuaterneCount GetRecommendedNumQuaterna() const;
		
		void Reset();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Unit * _units[num_units];
		QuaterneCount _current_num_quaterne;
	};
}
