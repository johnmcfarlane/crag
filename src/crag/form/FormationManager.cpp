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
#include "Mesh.h"

#include "form/Node/NodeBuffer.h"
#include "form/scene/ForEachIntersection.h"
#include "form/scene/SceneThread.h"

#include "sim/axes.h"
#include "sim/Observer.h"

#include "gfx/Pov.h"

#include "glpp/glpp.h"

#include "smp/ForEach.h"

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

	//CONFIG_DEFINE (enable_multithreding, bool, true);
	CONFIG_DEFINE (enable_dynamic_origin, bool, true);
	
	CONFIG_DEFINE (formation_sphere_collision_detail_factor, float, 10.f);

	STAT (num_polys, int, .05f);
	STAT (num_quats_used, int, 0.15f);
	STAT (mesh_generation, bool, .206f);
	STAT (dynamic_origin, bool, .206f);
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
		// TODO: This belongs in the OpenGL thread.
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices);
		mbo.Unbind();
		
		// TODO: Is there a 1:1 Mesh/Scene mapping? If so, Mesh should live in Scene.
		MeshDoubleBuffer::value_type & mesh = meshes[index];
		mesh = new Mesh (form::NodeBuffer::max_num_verts, static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f));
	}
	
	Assert(singleton == nullptr);
	singleton = this;
}

form::FormationManager::~FormationManager()
{
	Assert(singleton == this);
	singleton = nullptr;
	
	for (int index = 0; index < 2; ++ index)
	{
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Deinit();
		
		delete meshes [index];
	}
}

#if defined(VERIFY)
void form::FormationManager::Verify() const
{
	VerifyObject(scenes.front());
	VerifyObject(scenes.back());
}
#endif

void form::FormationManager::OnMessage(smp::TerminateMessage const & message)
{
}

void form::FormationManager::OnMessage(AddFormationMessage const & message)
{
	AddFormation(message.formation);
}

void form::FormationManager::OnMessage(RemoveFormationMessage const & message)
{
	RemoveFormation(message.formation);
}

void form::FormationManager::OnMessage(sim::SetCameraMessage const & message)
{
	SetCameraPos(message.projection);
}

void form::FormationManager::Run()
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(-1);
	
	while (ProcessMessages()) 
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

// TODO: Paralellize scenes[n].AddFormation/RemoveFormation
void form::FormationManager::AddFormation(form::Formation & formation)
{
	Assert(formation_set.find(& formation) == formation_set.end());
	formation_set.insert(& formation);
	scenes[0].AddFormation(formation);
	scenes[1].AddFormation(formation);
}

void form::FormationManager::RemoveFormation(form::Formation & formation)
{
	Assert(formation_set.find(& formation) != formation_set.end());
	formation_set.erase(& formation);
	scenes[0].RemoveFormation(formation);
	scenes[1].RemoveFormation(formation);
}

void form::FormationManager::ForEachIntersection(sim::Sphere3 const & sphere, Formation const & formation, IntersectionFunctor & functor) const
{
	Scene const & scene = GetVisibleScene();
	Polyhedron const * polyhedron = scene.GetPolyhedron(formation);
	if (polyhedron == nullptr)
	{
		Assert(false);
		return;
	}
	
	sim::Vector3 const & origin = scene.GetOrigin();
	form::Vector3 relative_formation_position(form::SimToScene(formation.position, origin));
	form::Sphere3 relative_sphere(form::SimToScene(sphere.center, origin), sphere.radius);
	
	NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	node_buffer.LockTree();
	
	float min_parent_score = node_buffer.GetMinParentScore() * formation_sphere_collision_detail_factor;
	form::ForEachIntersection(* polyhedron, relative_formation_position, relative_sphere, origin, functor, min_parent_score);
	
	node_buffer.UnlockTree();
}

void form::FormationManager::SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta)
{
	float ratio = static_cast<float>(frame_delta / target_frame_delta);
	regulator.SampleFrameRatio(ratio);
}

void form::FormationManager::ResetRegulator()
{
	regulator.Reset();
}

void form::FormationManager::ToggleSuspended()
{
	suspend_flag = ! suspend_flag;
	
	if (suspend_flag)
	{
		suspend_semaphore.Decrement();
	}
	else
	{
		suspend_semaphore.Increment();
	}
}

void form::FormationManager::ToggleMeshGeneration()
{
	enable_mesh_generation = ! enable_mesh_generation;
}

void form::FormationManager::ToggleDynamicOrigin()
{
	enable_dynamic_origin = ! enable_dynamic_origin;
}

void form::FormationManager::ToggleFlatShaded()
{
	bool flat_shaded = meshes[0]->GetProperties().flat_shaded;
	flat_shaded = ! flat_shaded;
	
	meshes[0]->GetProperties().flat_shaded = flat_shaded;
	meshes[1]->GetProperties().flat_shaded = flat_shaded;
}

void form::FormationManager::SetCameraPos(sim::CameraProjection const & projection)
{
	_camera_pos = axes::GetCameraRay(projection.pos, projection.rot);
}

void form::FormationManager::PollMesh()
{
	if (! enable_mesh_generation)
	{
		return;
	}
	
	if (! back_mesh_buffer_ready)
	{
		return;
	}
	
	Mesh & back_mesh = ref(meshes.back()); 
	
	mbo_buffers.back().Bind();
	mbo_buffers.back().Set(back_mesh);
	mbo_buffers.back().Unbind();
	
	mbo_buffers.flip();
	
	back_mesh_buffer_ready = false;

	STAT_SET (num_polys, mbo_buffers.front().GetNumPolys());

	// Used to live inside Render.
	STAT_SET (num_quats_used, scenes.front().GetNodeBuffer().GetNumQuaternaUsed());
	STAT_SET (mesh_generation, enable_mesh_generation);
	STAT_SET (dynamic_origin, enable_dynamic_origin);
}

void form::FormationManager::Render(gfx::Pov const & pov)
{
	if (mbo_buffers.front().GetNumPolys() <= 0)
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
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	front_buffer.Bind();
	front_buffer.Activate(pov);
	front_buffer.Draw();
	front_buffer.Deactivate();
	GLPP_VERIFY;
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
	
	GenerateMesh();

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
	
	Scene const & visible_scene = GetVisibleScene();
	
	Mesh & mesh = ref(meshes.back());
	visible_scene.GenerateMesh(mesh);
	
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
	NodeBuffer & visible_node_buffer = visible_scene.GetNodeBuffer();
	int current_num_quaterna = visible_node_buffer.GetNumQuaternaUsed();

	NodeBuffer & active_node_buffer = active_scene.GetNodeBuffer();
	active_node_buffer.SetNumQuaternaUsedTarget(current_num_quaterna);
}

void form::FormationManager::EndReset()
{
	VerifyObject(* this);
	is_in_reset_mode = false;
	scenes.flip();
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


form::FormationManager * form::FormationManager::singleton = nullptr;
