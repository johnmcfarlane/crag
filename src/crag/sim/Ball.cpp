//
//  Ball.cpp
//  crag
//
//  Created by John McFarlane on 6/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "Simulation.h"
#include "Universe.h"

#include "physics/SphericalBody.h"

#include "script/MetaClass.h"


namespace 
{
	CONFIG_DEFINE (ball_density, double, 1);

	CONFIG_DEFINE (ball_linear_damping, double, 0.025f);
	CONFIG_DEFINE (ball_angular_damping, double, 0.05f);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Ball script binding

DEFINE_SCRIPT_CLASS(sim, Ball);


////////////////////////////////////////////////////////////////////////////////
// sim::Ball script binding

using namespace sim;


Ball::Ball()
: _body(nullptr)
{
}

Ball::~Ball()
{
	_mesh.Deinit();
}

void Ball::Create(Ball & ball, PyObject & args)
{
	// construct ball
	new (& ball) Ball;
	
	// create message
	AddEntityMessage message = { ball, args };
	
	// send
	Simulation::SendMessage(message, true);
}

bool Ball::Init(PyObject & args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	double radius;
	
	if (! PyArg_ParseTuple(& args, "dddd", & center.x, & center.y, & center.z, & radius))
	{
		return false;
	}
	
	// physics
	physics::Engine & physics_engine = Simulation::Ref().GetPhysicsEngine();	
	_body = new physics::SphericalBody(physics_engine, true, radius);
	_body->SetPosition(center);
	_body->SetDensity(ball_density);
	_body->SetLinearDamping(ball_linear_damping);
	_body->SetAngularDamping(ball_angular_damping);
	
	// graphics
	InitMesh(radius);
	
	return true;
}

void Ball::Tick()
{
	// Gravity
	Universe const & universe = Simulation::Ref().GetUniverse();
	universe.ApplyGravity(* _body);
}

void Ball::Draw() const
{
	GLPP_VERIFY;

	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(0, 60);
	_mesh.Deactivate();
	_mesh.Unbind();
	
	GLPP_VERIFY;
}

void Ball::InitMesh(Scalar radius)
{
	// no prizes for guessing where I cribbed this code...
	Scalar const x = .525731112119133606 * radius;
	Scalar const z = .850650808352039932 * radius;
	
	static Vertex verts [] = 
	{
		{ Vector3(-x, .0,  z) },
		{ Vector3( x, .0,  z) },
		{ Vector3(-x, .0, -z) },
		{ Vector3( x, .0, -z) },
		{ Vector3(.0,  z,  x) },
		{ Vector3(.0,  z, -x) },
		{ Vector3(.0, -z,  x) },
		{ Vector3(.0, -z, -x) },
		{ Vector3( z,  x, .0) },
		{ Vector3(-z,  x, .0) },
		{ Vector3( z, -x, .0) },
		{ Vector3(-z, -x, .0) }
	};
	int num_verts = ARRAY_SIZE(verts);
	
	for (int i = 0; i < num_verts; ++ i)
	{
		verts[i].norm = verts[i].pos;
	}
	
	static GLuint tindices[20][3] = 
	{ 
		{0,4,1}, 
		{0,9,4}, 
		{9,5,4}, 
		{4,5,8}, 
		{4,8,1},    
		{8,10,1}, 
		{8,3,10}, 
		{5,3,8}, 
		{5,2,3}, 
		{2,7,3},    
		{7,10,3}, 
		{7,6,10}, 
		{7,11,6}, 
		{11,0,6}, 
		{0,1,6}, 
		{6,1,10}, 
		{9,0,11}, 
		{9,11,2}, 
		{9,2,5}, 
		{7,2,11} 
	};
	int num_indices = ARRAY_SIZE(tindices) * 3;
	
	_mesh.Init();
	_mesh.Bind();
	_mesh.Resize(num_verts, num_indices);
	_mesh.SetIbo(num_indices, tindices[0]);
	_mesh.SetVbo(num_verts, verts);
	_mesh.Unbind();
}

Vector3 const & Ball::GetPosition() const
{
	return _body->GetPosition();
}
