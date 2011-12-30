/*
 *  Observer.cpp
 *  Crag
 *
 *  Created by john on 5/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */ 

#include "pch.h"

#include "EntityFunctions.h"
#include "Observer.h"
#include "Simulation.h"

#include "physics/SphericalBody.h"

#include "geom/Transformation.h"
#include "geom/Vector4.h"

#include "core/app.h"
#include "core/ConfigEntry.h"

#include "script/MetaClass.h"

#include "gfx/Renderer.inl"
#include "gfx/object/Light.h"

#include "geom/MatrixOps.h"


using namespace sim;


namespace 
{
	CONFIG_DEFINE (observer_radius, double, .5);
	CONFIG_DEFINE (observer_density, double, 1);
	
	CONFIG_DEFINE (observer_speed_factor, double, 631);

	CONFIG_DEFINE (observer_linear_damping, double, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, double, 0.05f);
	CONFIG_DEFINE (observer_velocity_impulse, float, 0.002f);
	CONFIG_DEFINE (observer_torque_impulse, double, .0025f);

	CONFIG_DEFINE (observer_light_color, gfx::Color4f, gfx::Color4f(0.8f, 0.8f, 1.0f));
	CONFIG_DEFINE (observer_light_attenuation_a, float, 0.00000001f);
	CONFIG_DEFINE (observer_light_attenuation_b, float, 0.000f);
	CONFIG_DEFINE (observer_light_attenuation_c, float, 4.000f);
}


////////////////////////////////////////////////////////////////////////////////
// Observer script binding

namespace 
{
	PyObject * observer_add_rotation(PyObject * self, PyObject * args)
	{
		Vector3 rotations;
		if (! PyArg_ParseTuple(args, "ddd", & rotations.x, & rotations.y, & rotations.z))
		{
			// TODO: This is probably wront; use Py_RETURN_NONE instead.
			// TODO: In general, start throwing exceptions.
			return nullptr;
		}
		
		Observer & observer = Observer::GetRef(self);
		observer.AddRotation(rotations);
		
		Py_RETURN_NONE;
	}
	
	PyObject * observer_set_speed(PyObject * self, PyObject * args)
	{
		int speed_factor;
		if (! PyArg_ParseTuple(args, "i", & speed_factor))
		{
			return nullptr;
		}
		
		Observer & observer = Observer::GetRef(self);
		observer.SetSpeedFactor(speed_factor);
		
		Py_RETURN_NONE;
	}
}

DEFINE_SCRIPT_CLASS_BEGIN(sim, Observer)
	SCRIPT_CLASS_METHOD("add_rotation", observer_add_rotation, "Add some rotational impulse to the observer")
	SCRIPT_CLASS_METHOD("set_speed", observer_set_speed, "Set the impulse speed of the observer")
DEFINE_SCRIPT_CLASS_END


////////////////////////////////////////////////////////////////////////////////
// Observer	member definitions


Observer::Observer()
: Entity()
, speed(0)
, speed_factor(observer_speed_factor)
{
}

Observer::~Observer()
{
	// un-register with the renderer
	gfx::Daemon::Call<gfx::Uid>(_light_uid, & gfx::Renderer::OnRemoveObject);

	observer_speed_factor = static_cast<double>(speed_factor);
}

void Observer::Create(Observer & observer, PyObject & args)
{
	// construct observer
	new (& observer) Observer;
	
	// send creation message
	sim::Daemon::Call<sim::Entity *>(& observer, & args, & sim::Simulation::OnAddEntity);
}

bool Observer::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	if (! PyArg_ParseTuple(& args, "ddd", &center.x, &center.y, &center.z))
	{
		return false;
	}
	
	physics::Engine & physics_engine = simulation.GetPhysicsEngine();
	physics::SphericalBody * body = new physics::SphericalBody(physics_engine, true, observer_radius);
	SetSpeedFactor(1);
	
	body->SetDensity(observer_density);
	body->SetLinearDamping(observer_linear_damping);
	body->SetAngularDamping(observer_angular_damping);
	body->SetPosition(center);

	SetBody(body);
	
	impulses[0] = impulses[1] = Vector3::Zero();
	
	// register light with the renderer
	gfx::Light * light = new gfx::Light(observer_light_color, observer_light_attenuation_a, observer_light_attenuation_b, observer_light_attenuation_c);
	_light_uid = AddModelWithTransform(* light);
	
	return true;
}

void Observer::AddRotation(Vector3 const & angles)
{
	impulses[1] += angles;
}

void Observer::UpdateInput(Controller::Impulse const & impulse)
{
	Scalar velocity_impulse = observer_velocity_impulse * speed_factor;
	Vector3 const impulse_factors[2] = 
	{
		Vector3(velocity_impulse, velocity_impulse, velocity_impulse),
		Vector3(observer_torque_impulse, observer_torque_impulse, observer_torque_impulse),
	};
	
	Scalar inv_t = 1.f / Simulation::target_frame_seconds;

	for (int d = 0; d < 2; ++ d)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			Scalar factor = impulse.factors[d][axis];

			impulses[d][axis] += impulse_factors[d][axis] * factor * inv_t;
		}
	}
}

void Observer::SetSpeedFactor(int _speed_factor)
{
	speed_factor = static_cast<double>(pow(pow(10., .4), static_cast<double>((_speed_factor << 1) + 1)));
}

void Observer::Tick(Simulation & simulation)
{
	// Camera input.
	if (app::HasFocus()) 
	{
		UserInput ui;
		Controller::Impulse impulse = ui.GetImpulse();
		UpdateInput(impulse);
	}
	
	ApplyImpulse();
}

void Observer::UpdateModels() const
{
	Body const * body = GetBody();
	if (body == nullptr)
	{
		return;
	}

	Vector3 const & position = body->GetPosition();
	Matrix33 const & rotation = body->GetRotation();

	// Give renderer the new camera position.
	{
		Transformation transformation (position, rotation);
		gfx::Daemon::Call(transformation, & gfx::Renderer::OnSetCamera);
	}

	// Give renderer the new light position.
	{
		gfx::BranchNode::UpdateParams params = 
		{
			Transformation(position)
		};
		gfx::Daemon::Call(_light_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
	}
}

void Observer::ApplyImpulse()
{
	Body * body = GetBody();

	body->AddRelForce(impulses [0]);
	impulses[0] = Vector3::Zero();
	
	body->AddRelTorque(impulses [1]);
	impulses[1] = Vector3::Zero();
}
