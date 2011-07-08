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

namespace gfx
{
	template <typename VERTEX> class GeodesicSphere;
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
		void Draw(gfx::Pov const & pov) const;
	private:
		void SetMatrix(gfx::Pov const & pov) const;
		unsigned CalculateLod(gfx::Pov const & pov) const;
		
		virtual Vector3 const & GetPosition() const;
		
		void InitMesh(Scalar radius);
		void InitQuad();
		
		// types
		typedef gl::Vertex3dNorm MeshVertex;
		typedef gl::Vertex3dNorm QuadVertex;
		typedef gl::Vertex3dNorm Vertex;
		typedef gfx::GeodesicSphere<Vertex> GeodesicSphere;

		// variables
		physics::SphericalBody * _body;
		gl::Mesh<MeshVertex> _mesh;
		gl::Vbo<QuadVertex> _quad;
		std::vector<float> _lod_coefficients;
		
		static GLuint const num_lods = 4;
		static GLuint const lods[num_lods];
	};
}