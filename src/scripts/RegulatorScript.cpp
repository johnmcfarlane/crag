//
//  RegulatorScript.cpp
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RegulatorScript.h"

#include "applet/AppletInterface_Impl.h"

#include "form/Engine.h"

#include "gfx/Messages.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetOriginEvent.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


namespace 
{
	CONFIG_DEFINE (frame_duration_reaction_coefficient_base, float, 0.025f);
	CONFIG_DEFINE (frame_duration_reaction_coefficient_boost, float, .25f);
	CONFIG_DEFINE (frame_duration_reaction_coefficient_boost_half_life, float, 10.f);
	
	// TODO: Base it on full scene thread cycle.
	CONFIG_DEFINE (max_mesh_generation_period_proportion, float, .75f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
	
	typedef std::size_t QuaterneCount;
	const QuaterneCount invalid_num_quaterne = std::numeric_limits<QuaterneCount>::max();

	////////////////////////////////////////////////////////////////////////////////
	// Regulator class definitions

	// a regulator is something with an opinion about whether there should be more
	// or fewer quaterne being rendered
	class Regulator
	{
	public:
		virtual ~Regulator() { }
		virtual bool HasRecommendation() const = 0;
		virtual QuaterneCount GetRecommendedNumQuaterna() const = 0;
		virtual void PostRecommendation() = 0;
		virtual void BeginExit() = 0;
		virtual bool IsExitOk() const = 0;
	};

	typedef std::array<Regulator *, 2> RegulatorArray;

	////////////////////////////////////////////////////////////////////////////////
	// FrameRateRegulator definition
	
	// a regulator which recommends changes to the number of quaterne based on
	// whether the framerate is too high or low
	class FrameRateRegulator final 
		: public Regulator
		, ipc::Listener<applet::Engine, gfx::NumQuaterneSetMessage>
		, ipc::Listener<applet::Engine, gfx::FrameDurationSampledMessage>
	{
	public:
		FrameRateRegulator() 
		: _current_num_quaterne(invalid_num_quaterne)
		, _frame_duration_ratio_sum(_invalid_frame_rate_ratio)
		, _frame_duration_ratio_count(0)
		, _startup_time(app::GetTime())
		{
		}
		
	private:
		bool HasRecommendation() const
		{
			return _frame_duration_ratio_count > 0;
		}
		
		QuaterneCount GetRecommendedNumQuaterna() const
		{
			ASSERT(_frame_duration_ratio_count > 0);
			auto frame_duration_ratio_avg = _frame_duration_ratio_sum / _frame_duration_ratio_count;

			auto recommended_num_quaterne = MakeRecommendation(frame_duration_ratio_avg, _current_num_quaterne, _startup_time);

			return recommended_num_quaterne;
		}
		
		// called after GetRecommendedNumQuaterna
		void PostRecommendation()
		{
			_frame_duration_ratio_sum = 0;
			_frame_duration_ratio_count = 0;
		}
		
		void BeginExit()
		{
			ipc::Listener<applet::Engine, gfx::NumQuaterneSetMessage>::SetIsListening(false);
			ipc::Listener<applet::Engine, gfx::FrameDurationSampledMessage>::SetIsListening(false);
		}
		
		bool IsExitOk() const
		{
			return ! ipc::Listener<applet::Engine, gfx::NumQuaterneSetMessage>::IsBusy()
			&& ! ipc::Listener<applet::Engine, gfx::FrameDurationSampledMessage>::IsBusy();
		}
		
		void operator() (gfx::NumQuaterneSetMessage const & message)
		{
			_current_num_quaterne = QuaterneCount(message.num_quaterne);
		}

		// Take a sample of the frame ratio and apply it to the stored
		// running maximum since the last adjustment.
		void operator() (gfx::FrameDurationSampledMessage const & message)
		{
			// Validate input
			ASSERT(message.frame_duration_ratio == message.frame_duration_ratio);
			ASSERT(message.frame_duration_ratio >= 0);
			
			if (_current_num_quaterne == invalid_num_quaterne)
			{
				return;
			}

			auto frame_duration_ratio = message.frame_duration_ratio;
			frame_duration_ratio = std::max(frame_duration_ratio, std::numeric_limits<float>::min());
			
			_frame_duration_ratio_sum += frame_duration_ratio;
			++ _frame_duration_ratio_count;
		}
		
		static QuaterneCount MakeRecommendation(float const & frame_duration_ratio, QuaterneCount num_quaterne, core::Time startup_time)
		{
			ASSERT(frame_duration_ratio >= 0);
			ASSERT(! IsInf(frame_duration_ratio));
			
			float frame_duration_ratio_log = std::log(frame_duration_ratio);
			float frame_duration_ratio_exp = std::exp(frame_duration_ratio_log * - CalculateFrameRateReactionCoefficient(startup_time));
			
			auto raw_recommended_num_quaterne = static_cast<float>(num_quaterne);
			raw_recommended_num_quaterne *= frame_duration_ratio_exp;
			ASSERT(raw_recommended_num_quaterne > 0);
			
			int rounded_recommended_num_quaterne = std::max(1, static_cast<int>(raw_recommended_num_quaterne));
			QuaterneCount recommended_num_quaterne(rounded_recommended_num_quaterne);
			
			if (recommended_num_quaterne == num_quaterne)
			{
				if (raw_recommended_num_quaterne >= 0)
				{
					++ recommended_num_quaterne;
				}
				else 
				{
					-- recommended_num_quaterne;
				}
			}
			
			return recommended_num_quaterne;
		}
		
		// Annealing stage where changes to num quaterne are relatively high.
		static float CalculateFrameRateReactionCoefficient(core::Time startup_time)
		{
			core::Time sim_time = app::GetTime() - startup_time;
			float boost_factor = std::pow(.5f, static_cast<float>(sim_time) / frame_duration_reaction_coefficient_boost_half_life);
			float boost_summand = frame_duration_reaction_coefficient_boost * boost_factor;
			float base_summand = frame_duration_reaction_coefficient_base;
			return boost_summand + base_summand;
		}
		
		// variables
		QuaterneCount _current_num_quaterne;
		float _frame_duration_ratio_sum;
		int _frame_duration_ratio_count;
		core::Time _startup_time;
		
		// constants
		static float const _invalid_frame_rate_ratio;
	};

	// constants
	float const FrameRateRegulator::_invalid_frame_rate_ratio = -1.f;

	// a regulator which recommends changes to the number of quaterne based on
	// whether formation meshes are taking too long to generate
	class MeshGenerationRegulator final 
		: public Regulator
		, ipc::Listener<applet::Engine, gfx::MeshGenerationPeriodSampledMessage>
	{
		bool HasRecommendation() const
		{
			return _min_recommended_num_quaterne != invalid_num_quaterne;
		}
		
		QuaterneCount GetRecommendedNumQuaterna() const
		{
			return _min_recommended_num_quaterne;
		}

		void PostRecommendation()
		{
			_min_recommended_num_quaterne = invalid_num_quaterne;
		}
		
		void BeginExit()
		{
			ipc::Listener<applet::Engine, gfx::MeshGenerationPeriodSampledMessage>::SetIsListening(false);
		}

		bool IsExitOk() const
		{
			return ! ipc::Listener<applet::Engine, gfx::MeshGenerationPeriodSampledMessage>::IsBusy();
		}

		void operator() (gfx::MeshGenerationPeriodSampledMessage const & message)
		{
			// Validate input
			ASSERT(message.mesh_generation_period == message.mesh_generation_period);
			ASSERT(message.mesh_generation_period >= 0);
	
			auto max_mesh_generation_period = form::Engine::Daemon::ShutdownTimeout() * max_mesh_generation_period_proportion;
			if (message.mesh_generation_period < max_mesh_generation_period)
			{
				_min_recommended_num_quaterne = invalid_num_quaterne - 1;
				return;
			}
			
			QuaterneCount recommended_num_quaterne(std::max(static_cast<int>(message.num_quaterne * max_mesh_generation_reaction_coefficient) - 1, 0));
			if (recommended_num_quaterne < _min_recommended_num_quaterne || _min_recommended_num_quaterne == 0)
			{
				_min_recommended_num_quaterne = recommended_num_quaterne;
			}
		}
		
	private:
		// variables
		QuaterneCount _min_recommended_num_quaterne;
	};
}

////////////////////////////////////////////////////////////////////////////////
// script::RegulatorScript member definitions

void Tick(RegulatorArray const & regulators)
{
	for (auto regulator : regulators)
	{
		if (! regulator->HasRecommendation())
		{
			return;
		}
	}
	
	auto worst_num_quaterne = std::numeric_limits<std::size_t>::max();

	for (auto regulator : regulators)
	{
		auto recommended_num_quaterne = regulator->GetRecommendedNumQuaterna();
		
		worst_num_quaterne = std::min(worst_num_quaterne, recommended_num_quaterne);

		regulator->PostRecommendation();
	}

	form::Daemon::Call([worst_num_quaterne] (form::Engine & engine) 
	{
		engine.OnSetRecommendedNumQuaterne(worst_num_quaterne);
	});
}

void RegulatorScript(applet::AppletInterface & applet_interface)
{
	FrameRateRegulator frame_rate_regulator;
	MeshGenerationRegulator mesh_generation_regulator;
	RegulatorArray const regulators =
	{ {
		& frame_rate_regulator,
		& mesh_generation_regulator
	} };

	while (! applet_interface.GetQuitFlag())
	{
		Tick(regulators);

		applet_interface.Sleep(0.25);
	}

	for (auto regulator : regulators)
	{
		regulator->BeginExit();
	}
	
	applet_interface.WaitFor([& regulators] () -> bool 
	{
		for (auto regulator : regulators)
		{
			if (regulator->IsExitOk())
			{
				return true;
			}
		}
		
		return false;
	});
}

