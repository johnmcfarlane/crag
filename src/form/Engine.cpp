//
// form/form::Engine.cpp
// Crag
//
// Created by john on 5/23/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "Engine.h"

#include "Formation.h"

#include "form/node/NodeBuffer.h"

#include "sim/axes.h"

#include "gfx/Engine.h"
#include "gfx/Messages.h"
#include "gfx/object/FormationMesh.h"
#include "gfx/SetCameraEvent.h"

#include "core/app.h"
#include "core/profile.h"
#include "core/Statistics.h"


namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01f);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025f);
	PROFILE_DEFINE (mesh_generation_period, .01f);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01f);
	
	STAT (mesh_generation, bool, .206f);
	STAT (dynamic_origin, bool, .206f);
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
, _camera(Ray3::Zero())
, _scene(min_num_quaterne, max_num_quaterne)
{
	int max_num_verts = max_num_quaterne * form::NodeBuffer::num_nodes_per_quaterna;
	for (int num_meshes = 3; num_meshes > 0; -- num_meshes)
	{
		int max_num_tris = static_cast<int>(max_num_verts * 1.25f);
		Mesh & mesh = ref(new Mesh (max_num_verts, max_num_tris));
		_meshes.push_back(mesh);
	}
}

form::Engine::~Engine()
{
	while (! _meshes.empty())
	{
		Mesh & mesh = _meshes.front();
		_meshes.pop_front();
		delete & mesh;
	}
	
	ASSERT(_formations.empty());
}

#if defined(VERIFY)
void form::Engine::Verify() const
{
	VerifyObject(_scene);
}
#endif

void form::Engine::OnQuit()
{
	quit_flag = true;
}

void form::Engine::OnAddFormation(form::Formation & formation)
{
	ASSERT(_formations.find(& formation) == _formations.end());
	_formations.insert(& formation);
	_scene.AddFormation(formation, GetOrigin());
}

void form::Engine::OnRemoveFormation(form::Formation & formation)
{
	ASSERT(_formations.find(& formation) != _formations.end());
	_formations.erase(& formation);
	_scene.RemoveFormation(formation);
	
	delete & formation;
}

void form::Engine::OnSetMesh(Mesh & mesh)
{
	_meshes.push_back(mesh);
}

void form::Engine::operator() (gfx::SetCameraEvent const & event)
{
	_camera = axes::GetCameraRay(event.transformation);
}

void form::Engine::OnSetOrigin(geom::abs::Vector3 const & new_origin)
{
	_pending_origin_request = true;

	geom::abs::Vector3 camera_pos = geom::RelToAbs(_camera.position, GetOrigin());
	_camera.position = geom::AbsToRel(camera_pos, new_origin);
}

void form::Engine::EnableAdjustNumQuaterna(bool enabled)
{
	_enable_adjust_num_quaterna = enabled;
}

void form::Engine::OnSetRecommendedNumQuaterne(int recommented_num_quaterne)
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
	_mesh.Create(max_num_quaterne);
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

	// stop listening for SetCameraEvent
	SetIsListening(false);
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
	
	VerifyObject(* this);
}

void form::Engine::TickScene()
{
	PROFILE_TIMER_BEGIN(t);
	
	_scene.Tick(_camera);
	
	PROFILE_SAMPLE(scene_tick_per_quaterna, PROFILE_TIMER_READ(t) / _scene.GetNodeBuffer().GetNumQuaternaUsed());
	PROFILE_SAMPLE(scene_tick_period, PROFILE_TIMER_READ(t));
}

void form::Engine::AdjustNumQuaterna()
{
	if (! _enable_adjust_num_quaterna)
	{
		return;
	}
	
	// limit the range of quaterne counts
	Clamp(_requested_num_quaterne, int(min_num_quaterne), int(max_num_quaterne));
	
	// apply the recommended number
	NodeBuffer & active_buffer = _scene.GetNodeBuffer();
	active_buffer.SetNumQuaternaUsedTarget(_requested_num_quaterne);
}

void form::Engine::GenerateMesh()
{
	// get an available mesh
	Mesh * mesh = PopMesh();
	if (mesh == nullptr)
	{
		return;
	}
	
	// build it
	_scene.GenerateMesh(* mesh, GetOrigin());
	
	// sent it to the FormationSet object
	_mesh.Call([mesh] (gfx::FormationMesh & formation_mesh) {
		formation_mesh.SetMesh(mesh);
	});
	
	// record timing information
	core::Time t = app::GetTime();
	core::Time last_mesh_generation_period = t - mesh_generation_time;
	mesh_generation_time = t;

	// broadcast timing information
	gfx::MeshGenerationPeriodSampledMessage message = { last_mesh_generation_period };
	Daemon::Broadcast(message);
	
	// Sample the information for statistical output.
	PROFILE_SAMPLE(mesh_generation_per_quaterna, last_mesh_generation_period / _scene.GetNodeBuffer().GetNumQuaternaUsed());
	PROFILE_SAMPLE(mesh_generation_period, last_mesh_generation_period);
	
	smp::Yield();
}

form::Mesh * form::Engine::PopMesh()
{
	if (_meshes.empty())
	{
		return nullptr;
	}
	
	Mesh & mesh = _meshes.front();
	_meshes.pop_front();
	return & mesh;
}

void form::Engine::OnOriginReset()
{
	auto& node_buffer = _scene.GetNodeBuffer();
	int num_quaterna = node_buffer.GetNumQuaternaUsed();
	_scene.OnOriginReset(GetOrigin());
	node_buffer.SetNumQuaternaUsedTarget(num_quaterna);

	while (node_buffer.GetNumQuaternaUsed() < num_quaterna)
	{
		TickScene();
		ASSERT(node_buffer.GetNumQuaternaUsedTarget() == num_quaterna);
	}
}

bool form::Engine::IsGrowing() const
{
	NodeBuffer const & node_buffer = _scene.GetNodeBuffer();
	
	int num_used = node_buffer.GetNumQuaternaUsed();
	int num_used_target = node_buffer.GetNumQuaternaUsedTarget();
	
	return num_used < num_used_target;
}
