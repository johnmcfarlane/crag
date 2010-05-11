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

#include "sim/Defs.h"


namespace form 
{
	class Mesh;
	class Node;
	
	// TODO: Better name
	class Formation
	{
	public:
		Formation(class ShaderFactory const & init_shader_factory, float init_scale);	
		virtual ~Formation();
		
		float GetScale() const { return scale; }
		
		// heavy lifting
		void SetPosition(Vector3f const & init_position);
		void SetCameraPos(Vector3f const & camera_pos, Vector3f const & camera_dir);
		
		// collision
		void GenerateCollisionMesh(Mesh & mesh, sim::Sphere3 const & sphere) const;
		
	private:
		void Clear();
		void DeinitNode(Node & node);
		
	public:
		//DUMP_OPERATOR_DECLARATION(Formation);
		
#if VERIFY
		void Verify() const;
		void VerifyNode(class Node const & r) const;
		int CountNumNodes(Node const * node) const;
#endif
		
		Vector3d position;
		ShaderFactory const & shader_factory;
		float scale;
		int seed;
	};

}

