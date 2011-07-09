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

#include "gfx/Scene.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (ball_density, double, 1);

	CONFIG_DEFINE (ball_linear_damping, double, 0.005f);
	CONFIG_DEFINE (ball_angular_damping, double, 0.00005f);
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
	InitMesh();
	
	return true;
}

void Ball::Tick()
{
	// Gravity
	Universe const & universe = Simulation::Ref().GetUniverse();
	universe.ApplyGravity(* _body);
}

void Ball::Draw(gfx::Scene const & scene) const
{
	GLPP_VERIFY;
	
	gfx::Pov const & pov = scene.GetPov();
	
	// Calculate the LoD.
	unsigned lod = CalculateLod(pov);
	
	// Set the matrix.
	SetMatrix(pov);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	Scalar radius = _body->GetRadius();
	float scale = _lod_coefficients[lod] * radius;
	gl::Scale(scale, scale, scale);
	
	// Select the correct range of indices, given the LoD.
	unsigned faces_begin = lod ? GeodesicSphere::TotalNumFaces(lod - 1) : 0;
	unsigned faces_num = GeodesicSphere::LodNumFaces(lod);
	unsigned indices_begin = faces_begin * 3;
	unsigned indices_num = faces_num * 3;

	// Perform the draw calls.
	_mesh.Bind();
	_mesh.Activate();
	_mesh.Draw(GL_TRIANGLES, indices_num, indices_begin);
	_mesh.Deactivate();
	_mesh.Unbind();
	
	GLPP_VERIFY;
}

void Ball::SetMatrix(gfx::Pov const & pov) const
{
	gfx::Pov entity_pov (pov);
	entity_pov.pos = pov.pos - _body->GetPosition();
	sim::Matrix4 model_view_matrix = entity_pov.CalcModelViewMatrix();
	sim::Matrix4 ball_rotation;
	//_body->GetRotation(ball_rotation);
	//model_view_matrix = ball_rotation * model_view_matrix;
	
	gl::MatrixMode(GL_MODELVIEW);
	gl::LoadMatrix(model_view_matrix.GetArray());	
}

unsigned Ball::CalculateLod(gfx::Pov const & pov) const
{
	sim::Vector3 relative_position = pov.pos - _body->GetPosition();
	Scalar radius = _body->GetRadius();
	
	Scalar mn1 = 1500;
	Scalar inv_distance = Power(LengthSq(relative_position), -0.5);
	int lod = int(Power(mn1 * radius * inv_distance, .25));
	Clamp(lod, 1, 5);
	-- lod;
	
	return lod;
}

void Ball::InitMesh()
{
	GeodesicSphere source(5);
	
	_lod_coefficients = source.GetCoefficients();
	
	GeodesicSphere::VertexVector & verts = source.GetVerts();
	for (GeodesicSphere::VertexVector::iterator v = verts.begin(); v != verts.end(); ++ v)
	{
		v->norm = v->pos;
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
