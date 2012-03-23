//
// form/FormationManager.cpp
// Crag
//
// Created by john on 5/23/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "FormationManager.h"

#include "Formation.h"

#include "form/node/NodeBuffer.h"
#include "form/scene/RegulatorScript.h"

#include "sim/axes.h"

#include "gfx/Color.h"
#include "gfx/Renderer.h"
#include "gfx/object/FormationMesh.h"

#include "script/ScriptThread.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/profile.h"
#include "core/Statistics.h"


namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01f);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025f);
	PROFILE_DEFINE (mesh_generation_period, .01f);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01f);
	
	CONFIG_DEFINE (enable_dynamic_origin, bool, true);
	
	STAT (mesh_generation, bool, .206f);
	STAT (dynamic_origin, bool, .206f);
}


////////////////////////////////////////////////////////////////////////////////
// FormationManager member definitions

form::FormationManager::FormationManager()
: quit_flag(false)
, suspend_flag(false)
, enable_mesh_generation(true)
, flat_shaded_flag(false)
, mesh_generation_time(app::GetTime())
, _regulator_enabled(true)
, _recommented_num_quaterne(0)
, _camera_pos(sim::Ray3::Zero())
{
	smp::SetThreadPriority(-1);
	smp::SetThreadName("Formation");
	
	for (int num_meshes = 3; num_meshes > 0; -- num_meshes)
	{
		int max_num_verts = form::NodeBuffer::max_num_verts;
		int max_num_tris = static_cast<int>(max_num_verts * 1.25f);
		Mesh & mesh = ref(new Mesh (max_num_verts, max_num_tris));
		_meshes.push_back(mesh);
	}
}

form::FormationManager::~FormationManager()
{
	DEBUG_MESSAGE("FormationManager has %d meshes.", (int)_meshes.size());
	
	while (! _meshes.empty())
	{
		Mesh & mesh = _meshes.front();
		_meshes.pop_front();
		delete & mesh;
	}
	
	ASSERT(_formations.empty());
}

#if defined(VERIFY)
void form::FormationManager::Verify() const
{
	VerifyObject(scenes.front());
	VerifyObject(scenes.back());
}
#endif

void form::FormationManager::OnQuit()
{
	quit_flag = true;
}

void form::FormationManager::OnAddFormation(form::Formation * const & formation)
{
	ASSERT(_formations.find(formation) == _formations.end());
	_formations.insert(formation);
	scenes[0].AddFormation(* formation);
	scenes[1].AddFormation(* formation);
}

void form::FormationManager::OnRemoveFormation(form::Formation * const & formation)
{
	ASSERT(_formations.find(formation) != _formations.end());
	_formations.erase(formation);
	scenes[0].RemoveFormation(* formation);
	scenes[1].RemoveFormation(* formation);
	
	delete formation;
}

void form::FormationManager::OnSetMesh(Mesh * const & mesh)
{
	_meshes.push_back(* mesh);
}

void form::FormationManager::OnSetCamera(sim::Transformation const & transformation)
{
	SetCamera(transformation);
}

void form::FormationManager::OnRegulatorSetEnabled(bool const & enabled)
{
	_regulator_enabled = enabled;
}

void form::FormationManager::OnSetRecommendedNumQuaterne(int const & recommented_num_quaterne)
{
	_recommented_num_quaterne = recommented_num_quaterne;
}

void form::FormationManager::OnToggleSuspended()
{
	suspend_flag = ! suspend_flag;
}

void form::FormationManager::OnToggleMeshGeneration()
{
	enable_mesh_generation = ! enable_mesh_generation;
}

void form::FormationManager::OnToggleDynamicOrigin()
{
	enable_dynamic_origin = ! enable_dynamic_origin;
}

void form::FormationManager::OnToggleFlatShaded()
{
	flat_shaded_flag = ! flat_shaded_flag;
}

void form::FormationManager::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	_regulator_handle.Create();
	
	// register with the renderer
	_mesh.Create(_regulator_handle);
	gfx::Daemon::Call(& gfx::Renderer::OnSetParent, _mesh.GetUid(), gfx::Uid());
	
	while (! quit_flag) 
	{
		message_queue.DispatchMessages(* this);

		if (! suspend_flag)
		{
			Tick();
			
			message_queue.DispatchMessages(* this);
			
			GenerateMesh();
			BroadcastFormationUpdates();
		}
	}
	
	// un-register with the renderer
	_mesh.Destroy();
	
	// destroy regulator
	_regulator_handle.Destroy();
}

// lock visible node tree for reading
void form::FormationManager::LockTree()
{
	Scene const & scene = GetVisibleScene();
	NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	node_buffer.ReadLockTree();
}

// unlock visible node tree for reading
void form::FormationManager::UnlockTree()
{
	Scene const & scene = GetVisibleScene();
	NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	node_buffer.ReadUnlockTree();
}

form::Scene const & form::FormationManager::OnTreeQuery() const
{
	return GetVisibleScene();
}

void form::FormationManager::SetCamera(sim::Transformation const & transformation)
{
	_camera_pos = axes::GetCameraRay(transformation);
}

// The tick function of the scene thread. 
// This functions gets called repeatedly either in the scene thread - if there is on -
// or in the main thread as part of the main render/simulation iteration.
void form::FormationManager::Tick()
{
	bool reset_origin_flag = enable_dynamic_origin && ! IsOriginOk();
	
	if (reset_origin_flag) 
	{
		BeginReset();
	}
	
	if (! reset_origin_flag)
	{
		AdjustNumQuaterna();
	}
	
	TickActiveScene();
	
	if (IsResetting())
	{
		if (! IsGrowing())
		{
			EndReset();
		}
	}
	
	//VerifyObject(* this);
}

void form::FormationManager::TickActiveScene()
{
	PROFILE_TIMER_BEGIN(t);
	
	Scene & active_scene = GetActiveScene();
	active_scene.SetCameraRay(_camera_pos);
	active_scene.Tick();
	
	PROFILE_SAMPLE(scene_tick_per_quaterna, PROFILE_TIMER_READ(t) / active_scene.GetNodeBuffer().GetNumQuaternaUsed());
	PROFILE_SAMPLE(scene_tick_period, PROFILE_TIMER_READ(t));
}

void form::FormationManager::AdjustNumQuaterna()
{
	if (! _regulator_enabled || IsResetting())
	{
		return;
	}
	
	// Calculate the regulator output.
	Clamp(_recommented_num_quaterne, int(NodeBuffer::min_num_quaterne), int(NodeBuffer::max_num_quaterne));
	
	// Apply the regulator output.
	NodeBuffer & active_buffer = GetActiveScene().GetNodeBuffer();
	active_buffer.SetNumQuaternaUsedTarget(_recommented_num_quaterne);
}

void form::FormationManager::GenerateMesh()
{
	if (IsResetting())
	{
		return;
	}
	
	// get an available mesh
	Mesh * mesh = PopMesh();
	if (mesh == nullptr)
	{
		return;
	}
	
	Scene const & visible_scene = GetVisibleScene();
	
	// build it
	mesh->GetProperties()._flat_shaded = flat_shaded_flag;
	visible_scene.GenerateMesh(* mesh);
	
	// sent it to the FormationSet object
	_mesh.Call(& gfx::FormationMesh::SetMesh, mesh);
	
	// record timing information
	Time t = app::GetTime();
	Time last_mesh_generation_period = t - mesh_generation_time;
	mesh_generation_time = t;

	// Pass timing information on to the regulator.
	_regulator_handle.Call<Time>(& form::RegulatorScript::SampleMeshGenerationPeriod, last_mesh_generation_period);
	
	// Sample the information for statistical output.
	PROFILE_SAMPLE(mesh_generation_per_quaterna, last_mesh_generation_period / GetActiveScene().GetNodeBuffer().GetNumQuaternaUsed());
	PROFILE_SAMPLE(mesh_generation_period, last_mesh_generation_period);
	
	smp::Yield();
}

void form::FormationManager::BroadcastFormationUpdates()
{
	for (FormationSet::const_iterator i = _formations.begin(); i != _formations.end(); ++ i)
	{
		Formation const & formation = * * i;
		formation.SendRadiusUpdateMessage();
	}
}

form::Mesh * form::FormationManager::PopMesh()
{
	if (_meshes.empty())
	{
		return nullptr;
	}
	
	Mesh & mesh = _meshes.front();
	_meshes.pop_front();
	return & mesh;
}

void form::FormationManager::BeginReset()
{
	ASSERT(! IsResetting());
	is_in_reset_mode = true;
	
	// Transfer the origin from the old scene to the new one.
	Scene & active_scene = GetActiveScene();
	Scene & visible_scene = GetVisibleScene();
	active_scene.SetOrigin(_camera_pos.position);
	
	// Transfer the quaterna count from the old buffer to the new one.
	NodeBuffer & visible_node_buffer = visible_scene.GetNodeBuffer();
	int current_num_quaterne = visible_node_buffer.GetNumQuaternaUsed();
	
	NodeBuffer & active_node_buffer = active_scene.GetNodeBuffer();
	active_node_buffer.SetNumQuaternaUsedTarget(current_num_quaterne);
}

void form::FormationManager::EndReset()
{
	VerifyObject(* this);
	
	is_in_reset_mode = false;
	
	Scene const & scene = GetVisibleScene();
	NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	node_buffer.WriteLockTree();

	scenes.flip();

	node_buffer.WriteUnlockTree();

	VerifyObject(* this);
}

form::Scene & form::FormationManager::GetActiveScene()
{
	return IsResetting() ? scenes.back() : scenes.front();
}

form::Scene const & form::FormationManager::GetActiveScene() const
{
	return IsResetting() ? scenes.back() : scenes.front();
}

form::Scene & form::FormationManager::GetVisibleScene()
{
	return scenes.front();
}

form::Scene const & form::FormationManager::GetVisibleScene() const
{
	return scenes.front();
}

// Returns false if a new origin is needed.
bool form::FormationManager::IsOriginOk() const
{
	if (IsGrowing() || IsResetting())
	{
		// Still making the last one
		return true;
	}
	
	return scenes.front().IsOriginOk();
}

bool form::FormationManager::IsGrowing() const
{
	Scene const & active_scene = GetActiveScene();
	NodeBuffer const & node_buffer = active_scene.GetNodeBuffer();
	
	int num_used = node_buffer.GetNumQuaternaUsed();
	int num_used_target = node_buffer.GetNumQuaternaUsedTarget();
	
	return num_used < num_used_target;
}

bool form::FormationManager::IsResetting() const
{
	return is_in_reset_mode;
}


form::Daemon * form::FormationManager::singleton = nullptr;
