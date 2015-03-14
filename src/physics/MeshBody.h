//
//  physics/MeshBody.h
//  crag
//
//  Created by John McFarlane on 2014-01-14.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"

namespace physics
{
	class Engine;
	
	class MeshBody : public Body
	{
	public:
		MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh, Scalar volume);
		MeshBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Mesh const & mesh);
		~MeshBody();

	protected:
		bool OnCollision(Body & that_body, ContactInterface & contact_interface) override;

		void DebugDraw() const override;

		MeshData _mesh_data;
		Scalar _bounding_radius;
#if defined(CRAG_DEBUG)
		int _num_triangles;
#endif
	};
	
}
