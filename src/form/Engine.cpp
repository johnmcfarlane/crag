//
// form/Engine.cpp
// Crag
//
// Created by john on 5/23/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "Engine.h"

#include "Formation.h"

#include "Surrounding.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/Messages.h"
#include "gfx/object/Surrounding.h"
#include "gfx/SetLodParametersEvent.h"
#include "gfx/SetOriginEvent.h"

#include "core/app.h"
#include "core/profile.h"
#include "core/Statistics.h"

#include "geom/Intersection.h"

using namespace form;

namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01f);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025f);
	PROFILE_DEFINE (mesh_generation_period, .01f);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01f);
	
	STAT (mesh_generation, bool, .206f);
	STAT (dynamic_origin, bool, .206f);

	// the maximum size of formation-related buffers is limited by the maximum
	// value allowed in GLES index buffers (which are 16 in some cases)
	auto const max_allowed_num_verts = uintmax_t(std::numeric_limits<gfx::ElementIndex>::max());
	auto const max_desired_num_verts = uintmax_t(0x100000);
	auto const max_num_verts = int(std::min(max_allowed_num_verts, max_desired_num_verts));
	auto const max_num_tris = max_num_verts >> 1;
	auto const max_num_nodes = max_num_tris >> 1;
	auto const max_num_quaterne = max_num_nodes >> 2;
	auto const min_num_quaterne = std::min(1024, max_num_quaterne);
}


////////////////////////////////////////////////////////////////////////////////
// form::Engine member definitions

form::Engine::Engine()
: quit_flag(false)
, suspend_flag(false)
, enable_mesh_generation(true)
, mesh_generation_time(app::GetTime())
, _enable_adjust_num_quaterna(true)
, _requested_num_quaterne(0)
, _pending_origin_request(false)
, _lod_parameters({ Vector3::Zero(), 0.f })
, _origin(geom::abs::Vector3::Zero())
, _scene(min_num_quaterne, max_num_quaterne)
{
	for (int num_meshes = 3; num_meshes > 0; -- num_meshes)
	{
		auto mesh = std::make_shared<Mesh>();
		mesh->Reserve(max_num_verts, max_num_tris);
		_meshes.push(mesh);
	}
}

form::Engine::~Engine()
{
	ASSERT(_formations.empty());
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(form::Engine, self)
	CRAG_VERIFY(self._scene);
	CRAG_VERIFY(static_cast<super const &>(self));
	CRAG_VERIFY(static_cast<SetLodParametersListener const &>(self));
	CRAG_VERIFY(static_cast<SetOriginListener const &>(self));
CRAG_VERIFY_INVARIANTS_DEFINE_END

void form::Engine::OnQuit()
{
	quit_flag = true;
}

void form::Engine::OnAddFormation(form::Formation & formation)
{
	ASSERT(_formations.find(& formation) == _formations.end());
	_formations.insert(& formation);
	_scene.AddFormation(formation, _origin);
}

void form::Engine::OnRemoveFormation(form::Formation & formation)
{
	ASSERT(_formations.find(& formation) != _formations.end());
	_formations.erase(& formation);
	_scene.RemoveFormation(formation);
	
	delete & formation;
}

void form::Engine::OnSetMesh(std::shared_ptr<Mesh> const & mesh)
{
	_meshes.push(mesh);
}

void form::Engine::operator() (gfx::SetLodParametersEvent const & event)
{
	_lod_parameters = event.parameters;
}

void form::Engine::operator() (gfx::SetOriginEvent const & event)
{
	_pending_origin_request = true;

	_lod_parameters.center = geom::Convert(_lod_parameters.center, _origin, event.origin);
	_origin = event.origin;
}

void form::Engine::EnableAdjustNumQuaterna(bool enabled)
{
	_enable_adjust_num_quaterna = enabled;
}

void form::Engine::OnSetRecommendedNumQuaterne(std::size_t recommented_num_quaterne)
{
	_requested_num_quaterne = recommented_num_quaterne;
}

void form::Engine::OnToggleSuspended()
{
	suspend_flag = ! suspend_flag;
}

void form::Engine::OnToggleMeshGeneration()
{
	enable_mesh_generation = ! enable_mesh_generation;
}

void form::Engine::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	// register with the renderer
	_mesh.Create();
	auto mesh_handle = _mesh;
	gfx::Daemon::Call([mesh_handle](gfx::Engine & engine){
		engine.OnSetParent(mesh_handle.GetUid(), gfx::Uid());
	});
	
	while (! quit_flag) 
	{
		message_queue.DispatchMessages(* this);

		if (! suspend_flag)
		{
			Tick();
		}
	}
	
	// un-register with the renderer
	_mesh.Destroy();

	// stop listening for events
	SetLodParametersListener::SetIsListening(false);
	SetOriginListener::SetIsListening(false);
}

// The tick function of the scene thread. 
// This functions gets called repeatedly either in the scene thread - if there is on -
// or in the main thread as part of the main render/simulation iteration.
void form::Engine::Tick()
{
	if (_pending_origin_request) 
	{
		OnOriginReset();
		_pending_origin_request = false;
	}

	AdjustNumQuaterna();
	
	TickScene();
				
	GenerateMesh();
	
	CRAG_VERIFY(* this);
}

void form::Engine::TickScene()
{
	PROFILE_TIMER_BEGIN(t);
	
	_scene.Tick(_lod_parameters);
	
	PROFILE_SAMPLE(scene_tick_per_quaterna, PROFILE_TIMER_READ(t) / _scene.GetSurrounding().GetNumQuaternaUsed());
	PROFILE_SAMPLE(scene_tick_period, PROFILE_TIMER_READ(t));
}

void form::Engine::AdjustNumQuaterna()
{
	if (! _enable_adjust_num_quaterna)
	{
		return;
	}
	
	// limit the range of quaterne counts
	Clamp(_requested_num_quaterne, std::size_t(min_num_quaterne), std::size_t(max_num_quaterne));
	
	// apply the recommended number
	Surrounding & active_buffer = _scene.GetSurrounding();
	active_buffer.SetTargetNumQuaterna(_requested_num_quaterne);
}

void form::Engine::GenerateMesh()
{
	// get an available mesh
	std::shared_ptr<Mesh> mesh = PopMesh();
	if (! mesh)
	{
		return;
	}
	
	_scene.GenerateMesh(* mesh, _origin);
	
	// sent it to the FormationSet object
	_mesh.Call([mesh] (gfx::Surrounding & surrounding) {
		surrounding.SetMesh(mesh);
	});
	
	// record timing information
	::core::Time t = app::GetTime();
	::core::Time last_mesh_generation_period = t - mesh_generation_time;
	mesh_generation_time = t;

	// broadcast timing information
	gfx::MeshGenerationPeriodSampledMessage message = 
	{
		last_mesh_generation_period,
		_scene.GetSurrounding().GetNumQuaternaUsed()
	};
	Daemon::Broadcast(message);
	
	// Sample the information for statistical output.
	PROFILE_SAMPLE(mesh_generation_per_quaterna, last_mesh_generation_period / message.num_quaterne);
	PROFILE_SAMPLE(mesh_generation_period, last_mesh_generation_period);
	
	smp::Yield();
}

std::shared_ptr<Mesh> form::Engine::PopMesh()
{
	if (_meshes.empty())
	{
		return nullptr;
	}
	
	auto mesh = _meshes.top();
	_meshes.pop();
	return mesh;
}

void form::Engine::OnOriginReset()
{
	auto& surrounding = _scene.GetSurrounding();
	auto num_quaterna = surrounding.GetNumQuaternaUsed();
	_scene.OnOriginReset(_origin);
	surrounding.SetTargetNumQuaterna(num_quaterna);

	while (surrounding.GetNumQuaternaUsed() < num_quaterna)
	{
		TickScene();
		ASSERT(surrounding.GetTargetNumQuaterna() == num_quaterna);
	}
}

bool form::Engine::IsGrowing() const
{
	Surrounding const & surrounding = _scene.GetSurrounding();
	
	auto num_used = surrounding.GetNumQuaternaUsed();
	auto num_used_target = surrounding.GetTargetNumQuaterna();
	
	return num_used < num_used_target;
}
