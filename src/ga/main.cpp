//
//  main.cpp
//  crag
//
//  Created by John McFarlane on 2012-04-07.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "main.h"

#include "form/Engine.h"

#include "applet/AppletInterface_Impl.h"
#include "applet/Engine.h"

#include "sim/axes.h"
#include "sim/defs.h"
#include "sim/Engine.h"
#include "sim/EntityFunctions.h"
#include "sim/Firmament.h"
#include "sim/Planet.h"

#include "gfx/Engine.h"
#include "gfx/object/Light.h"

DECLARE_CLASS_HANDLE(sim, Planet);	// sim::PlanetHandle


using namespace ga;
typedef double Scalar;
typedef geom::Vector<Scalar, 3> Vector3;
typedef geom::Ray<Scalar, 3> Ray3;
typedef geom::Matrix<Scalar, 3, 3> Matrix33;
typedef geom::Transformation<Scalar> Transformation;

void ga::main(applet::AppletInterface & applet_interface)
{
	DEBUG_MESSAGE("ga::main");

	float planet_radius = 10000;
	//Scalar axis = planet_radius / sqrt(3.);
	//Vector3 surface(axis, axis, axis);
	Vector3 surface(0., 0., planet_radius);

	// origin
	Vector3 origin = surface * .8 * 0.;
	sim::Daemon::Call([origin] (sim::Engine & engine) {
		engine.SetOrigin(origin);
	});
	
	// planet
	sim::PlanetHandle planet_handle;
	sim::Sphere3 planet_sphere(sim::Vector3::Zero(), 10000);
	int random_seed = 3634;
	int num_craters = 0;
	planet_handle.Create(planet_sphere, random_seed, num_craters);
	
	// camera
	Vector3 camera_pos = surface * 1.01;
	Vector3 camera_forward = geom::Normalized(- camera_pos);
	Ray3 camera_ray(camera_pos, camera_forward);
	//Vector3 camera_up = geom::Normalized(Vector3(1, -1, -1));
	//Vector3 camera_up = geom::Normalized(Vector3(0, 0, 1));
	//Matrix33 camera_dir(axes::Rotation(camera_forward/*, camera_up*/));
	//sim::Transformation transformation(sim::Transformation::Matrix44::Identity());
	//sim::Transformation transformation(camera_pos, camera_dir);
	sim::Daemon::Call([camera_ray] (sim::Engine & engine) {
		engine.SetCamera(camera_ray);
	});
	
	sim::Transformation transformation(geom::Cast<sim::Scalar>(camera_pos));
	gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
		engine.OnSetCamera(transformation);
	});

	applet_interface.Sleep(2.f);
	
	// light
	gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 50000000000.f);
	gfx::BranchNodeHandle light = sim::AddModelWithTransform<gfx::Light>(color);
	gfx::Transformation light_transformation(gfx::Vector3(0., 0., 10.) * planet_radius);
	light.Call([light_transformation] (gfx::BranchNode & branch_node) {
		branch_node.SetTransformation(light_transformation);
	});

	// Add the skybox.
	sim::FirmamentHandle skybox;
	skybox.Create();
	gfx::Daemon::Call([skybox] (gfx::Engine & engine) {
		engine.OnSetParent(skybox.GetUid(), gfx::Uid());
	});
	
//	gfx::Light::InitData init_data =
//	{
//		100000000.,	// sun_orbit_distance
//		30000.	// sun_year
//	};
//	_sun.Create(init_data);
	
	applet_interface.Sleep(5.f);
	
	planet_handle.Destroy();
	
//	applet::Daemon::Call(& applet::Engine::OnQuit);
//
	SDL_Event quit_event;
	quit_event.type = SDL_QUIT;
	if (SDL_PushEvent(& quit_event) != 1)
	{
		DEBUG_BREAK("SDL_PushEvent returned error");
	}
}
