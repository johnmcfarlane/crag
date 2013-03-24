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

#include "core/app.h"
#include "core/ConfigEntry.h"


namespace 
{
	CONFIG_DEFINE (frame_duration_reaction_coefficient_base, float, 0.025f);
	CONFIG_DEFINE (frame_duration_reaction_coefficient_boost, float, 0.225f);
	CONFIG_DEFINE (frame_duration_reaction_coefficient_boost_half_life, float, 10.f);
	
	// TODO: Base it on full scene thread cycle.
	CONFIG_DEFINE (max_mesh_generation_period, float, 0.35f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
	
	typedef script::RegulatorScript::QuaterneCount QuaterneCount;
}


////////////////////////////////////////////////////////////////////////////////
// script::RegulatorScript::QuaterneCount member definitions

QuaterneCount::QuaterneCount() : _num(invalid().GetNumber()) 
{ 
	VerifyObject(* this);
}

QuaterneCount::QuaterneCount(int num_quaterne) : _num(num_quaterne) 
{ 
	VerifyObject(* this);
}

#if defined(VERIFY)
void QuaterneCount::Verify() const
{
	VerifyTrue(_num > 0);
}
#endif

QuaterneCount & QuaterneCount::operator=(QuaterneCount rhs)
{
	_num = rhs._num;
	
	VerifyObject(* this);
	return * this;
}

bool QuaterneCount::operator < (QuaterneCount const & rhs) const
{
	return GetNumber() < rhs.GetNumber(); 
}

int QuaterneCount::GetNumber() const 
{ 
	return _num; 
}

QuaterneCount QuaterneCount::max() 
{ 
	return QuaterneCount(std::numeric_limits<int>::max()); 
}

QuaterneCount QuaterneCount::invalid() 
{ 
	return QuaterneCount(std::numeric_limits<int>::max()); 
}

QuaterneCount min(QuaterneCount lhs, QuaterneCount rhs) 
{ 
	return (lhs.GetNumber() < rhs.GetNumber()) ? lhs : rhs; 
}

QuaterneCount & QuaterneCount::operator ++ () 
{
	++ _num;
	return * this; 
}

QuaterneCount & QuaterneCount::operator -- () 
{
	-- _num;
	return * this; 
}

bool operator == (QuaterneCount lhs, QuaterneCount rhs) 
{
	return lhs.GetNumber() == rhs.GetNumber(); 
}

bool operator != (QuaterneCount lhs, QuaterneCount rhs) 
{
	return lhs.GetNumber() != rhs.GetNumber(); 
}

std::ostream & operator << (std::ostream & out, QuaterneCount const & rhs)
{
	return out << rhs.GetNumber();
}


////////////////////////////////////////////////////////////////////////////////
// script::RegulatorScript::Unit class definitions

class script::RegulatorScript::Unit
{
public:
	virtual ~Unit() { }
	virtual void Reset() = 0;
	virtual QuaterneCount GetRecommendedNumQuaterna() const = 0;
};

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// FrameRateUnit definition
	
	class FrameRateUnit : public script::RegulatorScript::Unit
	{
	public:
		FrameRateUnit() 
		: _startup_time(app::GetTime())
		{
		}

		void SampleFrameDuration(float frame_duration_ratio, QuaterneCount num_quaterne)
		{
			frame_duration_ratio = std::max(frame_duration_ratio, std::numeric_limits<float>::min());
			QuaterneCount recommended_num_quaterne = MakeRecommendation(frame_duration_ratio, num_quaterne);
			_min_recommended_num_quaterne = std::min(_min_recommended_num_quaterne, recommended_num_quaterne);
		}
		
	private:
		virtual void Reset() override
		{
			_min_recommended_num_quaterne = QuaterneCount::invalid();
		}
		
		virtual QuaterneCount GetRecommendedNumQuaterna() const override
		{
			return _min_recommended_num_quaterne;
		}
		
		QuaterneCount MakeRecommendation(float const & frame_duration_ratio, QuaterneCount num_quaterne)
		{
			ASSERT(frame_duration_ratio >= 0);
			ASSERT(! IsInf(frame_duration_ratio));
			
			float frame_duration_ratio_log = std::log(frame_duration_ratio);
			float frame_duration_ratio_exp = std::exp(frame_duration_ratio_log * - CalculateFrameRateReactionCoefficient());
			
			float raw_recommended_num_quaterne = static_cast<float>(num_quaterne.GetNumber());
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
		float CalculateFrameRateReactionCoefficient() const
		{
			core::Time sim_time = app::GetTime() - _startup_time;
			float boost_factor = std::pow(.5f, static_cast<float>(sim_time) / frame_duration_reaction_coefficient_boost_half_life);
			float boost_summand = frame_duration_reaction_coefficient_boost * boost_factor;
			float base_summand = frame_duration_reaction_coefficient_base;
			return boost_summand + base_summand;
		}
		
	private:
		// variables
		QuaterneCount _min_recommended_num_quaterne;
		core::Time _startup_time;
	};
	
	class MeshGenerationUnit : public script::RegulatorScript::Unit
	{
		// functions
		virtual void Reset() override
		{
			_min_recommended_num_quaterne = QuaterneCount::invalid();
		}
		
		virtual QuaterneCount GetRecommendedNumQuaterna() const override
		{
			return _min_recommended_num_quaterne;
		}
		
	public:
		void SampleMeshGenerationPeriod(core::Time const & mesh_generation_period, QuaterneCount num_quaterne)
		{
			if (mesh_generation_period < max_mesh_generation_period)
			{
				return;
			}
			
			QuaterneCount recommended_num_quaterne(static_cast<int>(num_quaterne.GetNumber() * max_mesh_generation_reaction_coefficient) - 1);
			if (recommended_num_quaterne < _min_recommended_num_quaterne)
			{
				_min_recommended_num_quaterne = recommended_num_quaterne;
			}
		}
		
	private:
		// variables
		QuaterneCount _min_recommended_num_quaterne;
	};
}

using namespace script;

////////////////////////////////////////////////////////////////////////////////
// script::RegulatorScript member definitions

RegulatorScript::RegulatorScript()
{
	_units[frame_rate] = new FrameRateUnit;
	_units[mesh_generation] = new MeshGenerationUnit;
	
	Reset();
}

RegulatorScript::~RegulatorScript()
{
	delete _units[mesh_generation];
	delete _units[frame_rate];
}

void RegulatorScript::operator() (applet::AppletInterface & applet_interface)
{
	while (! applet_interface.GetQuitFlag())
	{
		QuaterneCount recommended_num_quaterne = GetRecommendedNumQuaterna();
		if (recommended_num_quaterne != QuaterneCount::invalid())
		{
			//DEBUG_MESSAGE("current:%d recommended:%d", _current_num_quaterne.GetNumber(), recommended_num_quaterne.GetNumber());
			form::Daemon::Call([recommended_num_quaterne] (form::Engine & engine) {
				engine.OnSetRecommendedNumQuaterne(recommended_num_quaterne.GetNumber());
			});
			Reset();
		}
		
		applet_interface.Sleep(0.25);
	}

	ipc::Listener<gfx::Engine, applet::Engine, gfx::NumQuaterneSetMessage>::BeginRelease();
	ipc::Listener<gfx::Engine, applet::Engine, gfx::FrameDurationSampledMessage>::BeginRelease();
	ipc::Listener<gfx::Engine, applet::Engine, gfx::MeshGenerationPeriodSampledMessage>::BeginRelease();

	applet_interface.WaitFor([this] () -> bool {
		return ipc::Listener<gfx::Engine, applet::Engine, gfx::NumQuaterneSetMessage>::IsReleased()
			&& ipc::Listener<gfx::Engine, applet::Engine, gfx::FrameDurationSampledMessage>::IsReleased()
			&& ipc::Listener<gfx::Engine, applet::Engine, gfx::MeshGenerationPeriodSampledMessage>::IsReleased();
	});
}

void RegulatorScript::operator() (gfx::NumQuaterneSetMessage message)
{
	_current_num_quaterne = QuaterneCount(message.num_quaterne);
}

// Take a sample of the frame ratio and apply it to the stored
// running maximum since the last adjustment.
void RegulatorScript::operator() (gfx::FrameDurationSampledMessage message)
{
	// Validate input
	ASSERT(message.frame_duration_ratio == message.frame_duration_ratio);
	ASSERT(message.frame_duration_ratio >= 0);
	
	if (_current_num_quaterne != QuaterneCount::invalid())
	{
		FrameRateUnit & frame_rate_unit = * static_cast<FrameRateUnit *>(_units[frame_rate]);
		frame_rate_unit.SampleFrameDuration(message.frame_duration_ratio, _current_num_quaterne);
	}
}

void RegulatorScript::operator() (gfx::MeshGenerationPeriodSampledMessage message)
{
	// Validate input
	ASSERT(message.mesh_generation_period == message.mesh_generation_period);
	ASSERT(message.mesh_generation_period >= 0);
	
	if (_current_num_quaterne != QuaterneCount::invalid())
	{
		MeshGenerationUnit & mesh_generation_unit = * static_cast<MeshGenerationUnit *>(_units[mesh_generation]);
		mesh_generation_unit.SampleMeshGenerationPeriod(message.mesh_generation_period, _current_num_quaterne);
	}
}

QuaterneCount RegulatorScript::GetRecommendedNumQuaterna() const
{
	// max also means invalid
	QuaterneCount min_recommended_num_quaterne = QuaterneCount::max();
	
	for (int i = 0; i != num_units; ++ i)
	{
		Unit & unit = ref(_units[i]);
		QuaterneCount recommended_num_quaterne = unit.GetRecommendedNumQuaterna();
		if (recommended_num_quaterne < min_recommended_num_quaterne)
		{
			min_recommended_num_quaterne = recommended_num_quaterne;
		}
	}
	
	return min_recommended_num_quaterne;
}

void RegulatorScript::Reset()
{
	_current_num_quaterne = QuaterneCount::invalid();
	_units[0]->Reset();
	_units[1]->Reset();
}
