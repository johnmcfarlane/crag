//
//  Ball.h
//  crag
//
//  Created by John McFarlane on 6/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"

#include "glpp/Mesh.h"
#include "glpp/Vbo_Types.h"


namespace physics
{
	class SphericalBody;
}


namespace sim
{
	class Ball : public Entity
	{
		DECLARE_SCRIPT_CLASS(Ball, Entity);
		
	public:
		Ball();
		~Ball();
		
		// Type-specific allocation via script.
		static void Create(Ball & ball, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(PyObject & args);
		
		void Tick();
		void Draw() const;
		
	private:
		virtual Vector3 const & GetPosition() const;
		
		void InitMesh(Scalar radius);
		
		// types
		typedef gl::Vertex3dNorm Vertex;

		// variables
		physics::SphericalBody * _body;
		gl::Mesh<Vertex> _mesh;
	};
}