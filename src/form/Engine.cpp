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

#include "form/Surrounding.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/Messages.h"
#include "gfx/object/FormationMesh.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetOriginEvent.h"

#include "core/app.h"
#include "core/profile.h"
#include "core/Statistics.h"

using namespace form;

#include "geom/Intersection.h"
namespace 
{
	PROFILE_DEFINE (scene_tick_period, .01f);
	PROFILE_DEFINE (scene_tick_per_quaterna, .0025f);
	PROFILE_DEFINE (mesh_generation_period, .01f);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .01f);
	
	STAT (mesh_generation, bool, .206f);
	STAT (dynamic_origin, bool, .206f);

#if defined(CRAG_USE_GLES)
	auto const max_num_verts = 0x10000;
#else
	auto const max_num_verts = 0x100000;
#endif
	auto const max_num_tris = max_num_verts >> 1;
	auto const max_num_nodes = max_num_tris >> 1;
	auto const max_num_quaterne = max_num_nodes >> 2;
	auto const min_num_quaterne = std::min(1024, max_num_quaterne);

	void GenerateShadows(Mesh & mesh, geom::Vector<double, 3> const & /*center*/, geom::Sphere<double, 3> const & light)
	{
		auto & vertex_buffer = mesh.GetVertices();
		auto const & index_buffer = mesh.GetIndices();
		
		for (auto index_iterator = std::begin(index_buffer); index_iterator != std::end(index_buffer);)
		{
			typedef double Scalar;
			typedef geom::Triangle<Scalar, 3> Triangle;
			typedef geom::Plane<Scalar, 3> Plane;

			ASSERT(index_iterator < std::end(index_buffer));
			auto const & a = vertex_buffer.GetArray()[* index_iterator ++];
			auto const & b = vertex_buffer.GetArray()[* index_iterator ++];
			auto const & c = vertex_buffer.GetArray()[* index_iterator ++];
			ASSERT(index_iterator <= std::end(index_buffer));

			Triangle triangle(geom::Cast<Scalar>(a.pos), geom::Cast<Scalar>(b.pos), geom::Cast<Scalar>(c.pos));
			Plane plane(triangle);
			if (geom::DotProduct(plane.normal, light.center - plane.position) > 0)
			{
				continue;
			}
			std::array<Plane, 3> sides;
			for (auto index = 0; index < 3; ++ index)
			{
				Triangle side_triangle(light.center, triangle.points[TriMod(index + 2)], triangle.points[TriMod(index + 1)]);
				sides[index] = side_triangle;
			}
			
			for (auto & vertex : vertex_buffer)
			{
				if (! vertex.col.a || &vertex == &a || &vertex == &b || &vertex == &c)
				{
					continue;
				}
				
				auto point = geom::Cast<Scalar>(vertex.pos);
				if (geom::Contains(sides[0], point) && geom::Contains(sides[1], point) && geom::Contains(sides[2], point))
				{
					if (! geom::Contains(plane, point))
					{
						vertex.col.a = 0;
					}
				}
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// form::Engine member definitions

form::Engine::Engine()
: quit_flag(false)
, suspend_flag(false)
, enable_mesh_generation(true)
, _shadows_enabled(false)
, mesh_generation_time(app::GetTime())
, _enable_adjust_num_quaterna(true)
, _requested_num_quaterne(0)
, _pending_origin_request(false)
, _camera(geom::rel::Ray3::Zero())
, _origin(geom::abs::Vector3::Zero())
, _scene(min_num_quaterne, max_num_quaterne)
{
	for (int num_meshes = 3; num_meshes > 0; -- num_meshes)
	{
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
	_scene.AddFormation(formation, _origin);
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
	auto camera_ray = gfx::GetCameraRay(event.transformation);
	_camera = geom::AbsToRel(camera_ray, _origin);
}

void form::Engine::operator() (gfx::SetOriginEvent const & event)
{
	_pending_origin_request = true;

	geom::abs::Vector3 camera_pos = geom::RelToAbs(_camera.position, _origin);
	_origin = event.origin;
	_camera.position = geom::AbsToRel(camera_pos, _origin);
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

void form::Engine::SetShadowsEnabled(bool shadows_enabled)
{
	_shadows_enabled = shadows_enabled;
}

bool form::Engine::GetShadowsEnabled() const
{
	return _shadows_enabled;
}

void form::Engine::SetShadowLight(geom::abs::Sphere3 const & shadow_light)
{
	_shadow_light = shadow_light;
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

	// stop listening for events
	SetCameraListener::SetIsListening(false);
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
	
	VerifyObject(* this);
}

void form::Engine::TickScene()
{
	PROFILE_TIMER_BEGIN(t);
	
	_scene.Tick(_camera);
	
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
	Mesh * mesh = PopMesh();
	if (mesh == nullptr)
	{
		return;
	}
	
	// build it
	_scene.GenerateMesh(* mesh, _origin);
	
	if (_shadows_enabled)
	{
		auto center = geom::AbsToRel<double>(geom::abs::Vector3::Zero(), _origin);
		auto light = geom::AbsToRel<double>(_shadow_light, _origin);
		GenerateShadows(* mesh, center, light);
	}
	
	// sent it to the FormationSet object
	_mesh.Call([mesh] (gfx::FormationMesh & formation_mesh) {
		formation_mesh.SetMesh(mesh);
	});
	
	// record timing information
	core::Time t = app::GetTime();
	core::Time last_mesh_generation_period = t - mesh_generation_time;
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
