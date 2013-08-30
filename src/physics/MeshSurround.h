//
//  MeshSurround.h
//  crag
//
//  Created by John on 2013-08-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace physics
{
	////////////////////////////////////////////////////////////////////////////////
	// MeshSurround class definition

	class MeshSurround
	{
		OBJECT_NO_COPY(MeshSurround);

		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef std::array<int, 3> Triangle;
		typedef std::vector<Vector3> VerticesVector;
		typedef std::vector<Triangle> TrianglesVector;
		typedef std::vector<Vector3> NormalsVector;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		MeshSurround();
		MeshSurround(MeshSurround && rhs);
		~MeshSurround();

		CollisionHandle GetCollisionHandle() const;

		void Enable();
		void Disable();

		void ClearData();
		void AddTriangle(Vector3 const & a, Vector3 const & b, Vector3 const & c, Vector3 const & normal);
		void RefreshData();

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		MeshData _mesh_data;
		CollisionHandle _collision_handle;
		VerticesVector _vertices;	// TODO: replace with node buffer
		TrianglesVector _triangles;
		NormalsVector _normals;
	};
}
