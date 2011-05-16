/*
 *  form/Formation.h
 *  Crag
 *
 *  Created by john on 6/2/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"


namespace form 
{
	class Mesh;
	class Node;
	

	// A formation is an individual element of the formation system.
	// It contains all the data necessary to create a positioned polyhedron.
	class Formation
	{
	public:
		Formation(class ShaderFactory const & init_shader_factory);	
		virtual ~Formation();
		
		// heavy lifting
		void SetPosition(Vector3f const & init_position);
		void SetCameraPos(Vector3f const & camera_pos, Vector3f const & camera_dir);
		
		// collision
		void GenerateCollisionMesh(Mesh & mesh, sim::Sphere3 const & sphere) const;
		
		//DUMP_OPERATOR_DECLARATION(Formation);
		
#if defined(VERIFY)
		void Verify() const;
		void VerifyNode(class Node const & r) const;
		int CountNumNodes(Node const * node) const;
#endif
		
		Vector3d position;
		ShaderFactory const & shader_factory;
		int seed;
	};

}

