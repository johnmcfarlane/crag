/*
 *  form/FormationManager.cpp
 *  Crag
 *
 *  Created by john on 5/23/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "FormationManager.h"

#include "Formation.h"
#include "Mesh.h"

#include "form/Node/NodeBuffer.h"
#include "form/scene/SceneThread.h"

#include "sim/Observer.h"

#include "gfx/Pov.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "core/profile.h"
#include "core/Statistics.h"
#include "core/TextureMapper.h"


namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025);
	PROFILE_DEFINE (mesh_generation_period, .01);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01);
		
	CONFIG_DEFINE (formation_emission, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_ambient, gfx::Color4f, gfx::Color4f(0.05f));
	CONFIG_DEFINE (formation_diffuse, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_specular, float, 0.0f);
	CONFIG_DEFINE (formation_shininess, float, 0.0f);

	CONFIG_DEFINE (enable_multithreding, bool, true);
	CONFIG_DEFINE (enable_dynamic_origin, bool, true);

	STAT (num_polys, int, .05);
	STAT (mesh_generation, bool, .206);
	STAT (dynamic_origin, bool, .206);
}


form::FormationManager::FormationManager()
: quit_flag(false)
, suspend_flag(false)
, enable_mesh_generation(true)
, back_mesh_buffer_ready(false)
, suspend_semaphore(1)
, mesh_generation_time(sys::GetTime())
{
	for (int index = 0; index < 2; ++ index)
	{
		// initialize mesh buffer
		MboDoubleBuffer::value_type & mbo = mesh_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices);
		
		// TODO: Is there a 1:1 Mesh/Scene mapping? If so, Mesh should live in Scene.
		MeshDoubleBuffer::value_type & mesh = meshes[index];
		mesh = new Mesh (form::NodeBuffer::max_num_verts, static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f));
	}
}

form::FormationManager::~FormationManager()
{
	delete meshes [0];
	delete meshes [1];
}

void form::FormationManager::Run()
{
	FUNCTION_NO_REENTRY;
	
//	// This sleep is really only here so that IsSceneThread/IsMainThread will work properly.
//	// (See notes in Thread.h for details.)
//	smp::Sleep(1);
	
	while (true) 
	{
		suspend_semaphore.Decrement();
		if (quit_flag)
		{
			break;
		}
		
		Tick();
		
		suspend_semaphore.Increment();
		if (quit_flag)
		{
			break;
		}
	}
}

void form::FormationManager::Exit()
{
	FUNCTION_NO_REENTRY;
	
	quit_flag = true;
}

void form::FormationManager::AddFormation(form::Formation & formation)
{
	ptr lock = GetLock();

	Assert(lock->formation_set.find(& formation) == lock->formation_set.end());
	lock->formation_set.insert(& formation);
	lock->scenes[0].AddFormation(formation);
	lock->scenes[1].AddFormation(formation);
}

void form::FormationManager::RemoveFormation(form::Formation & formation)
{
	ptr lock = GetLock();
	
	Assert(lock->formation_set.find(& formation) != lock->formation_set.end());
	lock->formation_set.erase(& formation);
	lock->scenes[0].RemoveFormation(formation);
	lock->scenes[1].RemoveFormation(formation);
}

void form::FormationManager::ForEachFormation(FormationFunctor & f) const
{
	ptr lock = GetLock();
	
	GetVisibleScene().ForEachFormation(f);
}

void form::FormationManager::SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta)
{
	FormationManager & instance = Get();
	
	float ratio = static_cast<float>(frame_delta / target_frame_delta);
	instance.regulator.SampleFrameRatio(ratio);
}

void form::FormationManager::ResetRegulator()
{
	FormationManager & instance = Get();
	
	instance.regulator.Reset();
}

void form::FormationManager::ToggleSuspended()
{
	FormationManager & instance = Get();
	
	instance.suspend_flag = ! instance.suspend_flag;
	
	if (instance.suspend_flag)
	{
		instance.suspend_semaphore.Decrement();
	}
	else
	{
		instance.suspend_semaphore.Increment();
	}
}

void form::FormationManager::ToggleMeshGeneration()
{
	FormationManager & instance = Get();
	
	instance.enable_mesh_generation = ! instance.enable_mesh_generation;
}

void form::FormationManager::ToggleDynamicOrigin()
{
	enable_dynamic_origin = ! enable_dynamic_origin;
}

void form::FormationManager::ToggleFlatShaded()
{
	FormationManager & instance = Get();
	
	bool flat_shaded = instance.meshes[0]->GetProperties().flat_shaded;
	flat_shaded = ! flat_shaded;
	
	instance.meshes[0]->GetProperties().flat_shaded = flat_shaded;
	instance.meshes[1]->GetProperties().flat_shaded = flat_shaded;
}

void form::FormationManager::SetCameraPos(sim::Ray3 const & camera_pos)
{
	FormationManager & f = Get();
	f._camera_pos = camera_pos;
}

void form::FormationManager::PollMesh()
{
	FormationManager & s = Get();
	
	if (! s.enable_mesh_generation)
	{
		return;
	}
	
	if (! s.back_mesh_buffer_ready)
	{
		return;
	}
	
	s.mesh_buffers.back().Set(* s.meshes.back());
	s.mesh_buffers.flip();
	s.mesh_buffers.back().Clear();
	s.back_mesh_buffer_ready = false;

	STAT_SET (num_polys, s.mesh_buffers.front().GetNumPolys());
}

void form::FormationManager::Render(gfx::Pov const & pov, bool color)
{
	RenderFormations(pov, color);
	
	// This is an arbitrary way to make sure this fn only gets called once per frame.
	if (color)
	{
		DebugStats();
	}
}

void form::FormationManager::RenderFormations(gfx::Pov const & pov, bool color) 
{
	FormationManager & singleton = Get();
	
	if (singleton.mesh_buffers.front().GetNumPolys() <= 0)
	{
		return;
	}
	
	// State
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular)));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess));
	GLPP_CALL(glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE));

	Assert(! gl::IsEnabled(GL_TEXTURE_2D));
	
	// Draw the mesh!
	form::MeshBufferObject const & front_buffer = singleton.mesh_buffers.front();
	front_buffer.BeginDraw(pov, color);
	front_buffer.Draw();
	front_buffer.EndDraw();
	GLPP_VERIFY;
	
	//gl::Enable(GL_COLOR_MATERIAL);
}

void form::FormationManager::DebugStats() 
{
	/* if (gfx::Debug::GetVerbosity() > .75) 
	{
		for (int i = 0; i < 2; ++ i) 
		{
			form::MeshBufferObject const * m = & mesh_buffers [i];
			gfx::Debug::out << m << ' ';
			if (m == & mesh_buffers.front()) 
			{
				gfx::Debug::out << 'f';
			}
			if (m == & mesh_buffers.back()) 
			{
				gfx::Debug::out << 'b';
			}
			gfx::Debug::out << '\n';
		}
	} */
	
	STAT_SET (mesh_generation, Get().enable_mesh_generation);
	STAT_SET (dynamic_origin, enable_dynamic_origin);
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

	ptr lock = GetLock();
	
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
	
	GenerateMesh();
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
	if (IsResetting())
	{
		return;
	}
	
	// Get the regulator input.
	NodeBuffer & active_buffer = GetActiveScene().GetNodeBuffer();
	int current_num_quaterna = active_buffer.GetNumQuaternaUsed();
	
	// Calculate the regulator output.
	int target_num_quaterna = regulator.GetAdjustedLoad(current_num_quaterna);
	Clamp(target_num_quaterna, int(NodeBuffer::min_num_quaterna), int(NodeBuffer::max_num_quaterna));
	
	// Apply the regulator output.
	active_buffer.SetNumQuaternaUsedTarget(target_num_quaterna);
}

void form::FormationManager::GenerateMesh()
{
	if (IsResetting() || back_mesh_buffer_ready)
	{
		return;
	}
	
	Scene & visible_scene = GetVisibleScene();
	
	visible_scene.GenerateMesh(* meshes.back());
	
	back_mesh_buffer_ready = true;
	
	sys::TimeType t = sys::GetTime();
	sys::TimeType last_mesh_generation_period = t - mesh_generation_time;
	regulator.SampleMeshGenerationPeriod(last_mesh_generation_period);
	mesh_generation_time = t;
	
	PROFILE_SAMPLE(mesh_generation_per_quaterna, last_mesh_generation_period / GetActiveScene().GetNodeBuffer().GetNumQuaternaUsed());
	PROFILE_SAMPLE(mesh_generation_period, last_mesh_generation_period);
}

void form::FormationManager::BeginReset()
{
	Assert(! IsResetting());
	is_in_reset_mode = true;
	
	// Transfer the origin from the old scene to the new one.
	Scene & active_scene = GetActiveScene();
	Scene & visible_scene = GetVisibleScene();
	active_scene.SetOrigin(_camera_pos.position);
	
	// Transfer the quaterna count from the old buffer to the new one.
	NodeBuffer & active_node_buffer = active_scene.GetNodeBuffer();
	NodeBuffer & visible_node_buffer = visible_scene.GetNodeBuffer();
	int current_num_quaterna = visible_node_buffer.GetNumQuaternaUsed();
	active_node_buffer.SetNumQuaternaUsedTarget(current_num_quaterna);
}

void form::FormationManager::EndReset()
{
	is_in_reset_mode = false;
	scenes.flip();
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
