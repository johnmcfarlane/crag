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

#include "gfx/GeodesicSphere.h"

#include "script/MetaClass.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
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
	
	// temp code to show off the lodding capability
	unsigned lod = int(2.5 + 2.49 * Sin(sys::GetTime()));
	assert(lod >= 0 && lod < 6);
	unsigned faces_begin = lod ? GeodesicSphere::TotalNumFaces(lod - 1) : 0;
	unsigned faces_num = GeodesicSphere::LodNumFaces(lod);

	// compensate for the smaller low-lod versions with scaling.
	float scale = _lod_coefficients[lod];
	gl::Scale(scale, scale, scale);
	
	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(faces_begin * 3, faces_num * 3);
	_mesh.Deactivate();
	_mesh.Unbind();
	
	GLPP_VERIFY;
}

void Ball::InitMesh(Scalar radius)
{
	GeodesicSphere source(5, radius);
	
	_lod_coefficients = source.GetCoefficients();
	
	GeodesicSphere::VertexVector & verts = source.GetVerts();
	float inverse_radius = 1.f / radius;
	for (GeodesicSphere::VertexVector::iterator v = verts.begin(); v != verts.end(); ++ v)
	{
		v->norm = v->pos * inverse_radius;
	}
	
	GeodesicSphere::FaceVector & faces = source.GetFaces();
	
	_mesh.Init();
	_mesh.Bind();

	_mesh.Resize(verts.size(), faces.size() * 3);
	_mesh.SetIbo(faces.size() * 3, faces[0]._indices);
	_mesh.SetVbo(verts.size(), & verts[0]);

	_mesh.Unbind();
}

Vector3 const & Ball::GetPosition() const
{
	return _body->GetPosition();
}
