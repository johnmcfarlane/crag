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
#include "form/scene/ForEachIntersection.h"
#include "form/scene/Mesh.h"
#include "form/scene/MeshBufferObject.h"

#include "sim/axes.h"
#include "sim/Observer.h"

#include "glpp/glpp.h"

#include "smp/ForEach.h"

#include "core/ConfigEntry.h"
#include "core/profile.h"
#include "core/Statistics.h"
#include "core/TextureMapper.h"


namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01f);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025f);
	PROFILE_DEFINE (mesh_generation_period, .01f);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01f);
		
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
, rendered_num_quaternia(0)
, _camera_pos(sim::Ray3::Zero())
{
	for (int index = 0; index < 2; ++ index)
	{
		// TODO: Is there a 1:1 Mesh/Scene mapping? If so, Mesh should live in Scene.
		MeshDoubleBuffer::value_type & mesh = meshes[index];
		mesh = new Mesh (form::NodeBuffer::max_num_verts, static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f));
	}

	smp::SetThreadPriority(-1);
	smp::SetThreadName("Formation");
}

form::FormationManager::~FormationManager()
{
	for (int index = 0; index < 2; ++ index)
	{
		delete meshes [index];
	}
	
	Assert(formation_set.empty());
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
	quit_flag = true;
}

void form::FormationManager::OnMessage(AddFormationMessage const & message)
{
	AddFormation(message.formation);
}

void form::FormationManager::OnMessage(RemoveFormationMessage const & message)
{
	RemoveFormation(message.formation);
	delete & message.formation;
}

void form::FormationManager::OnMessage(sim::SetCameraMessage const & message)
{
	SetCameraPos(message.projection);
}

void form::FormationManager::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(-1);
	smp::SetThreadName("Formation");
	
	while (true) 
	{
		message_queue.DispatchMessages(* this);
		
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
	form::Sphere3 relative_sphere(form::SimToScene(sphere.center, origin), form::Scalar(sphere.radius));
	
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

bool form::FormationManager::PollMesh(MeshBufferObject & mbo)
{
	STAT_SET (mesh_generation, enable_mesh_generation);
	STAT_SET (dynamic_origin, enable_dynamic_origin);

	if (! enable_mesh_generation)
	{
		return false;
	}
	
	if (! back_mesh_buffer_ready)
	{
		return false;
	}
	
	Mesh & back_mesh = ref(meshes.back()); 
	
	mbo.Bind();
	mbo.Set(back_mesh);
	mbo.Unbind();
	
	back_mesh_buffer_ready = false;

	STAT_SET (num_polys, mbo.GetNumPolys());

	// Used to live inside Render.
	STAT_SET (num_quats_used, scenes.front().GetNodeBuffer().GetNumQuaternaUsed());
	
	return true;
}

void form::FormationManager::Render()
{
	// When asking the regulator to adjust the number of quaterna.
	NodeBuffer & active_buffer = GetActiveScene().GetNodeBuffer();
	rendered_num_quaternia = active_buffer.GetNumQuaternaUsed();
	
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
	
	// Calculate the regulator output.
	int target_num_quaterna = regulator.GetAdjustedLoad(rendered_num_quaternia);
	Clamp(target_num_quaterna, int(NodeBuffer::min_num_quaterna), int(NodeBuffer::max_num_quaterna));
	
	// Apply the regulator output.
	NodeBuffer & active_buffer = GetActiveScene().GetNodeBuffer();
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
	
	smp::Sleep(0);
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


form::FormationManager::Daemon * form::FormationManager::singleton = nullptr;
