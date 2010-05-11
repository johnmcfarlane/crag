/*
 *  Model.h
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "RootNode.h"


namespace form
{
	class VertexBuffer;
	
	class Model
	{
	public:
		Model();
		Model(Model const & rhs);
		~Model();
		
		void SetShader(class Shader * init_shader);
		Shader & GetShader();
		
		enum 
		{
			NUM_ROOT_VERTICES = 4
		};
		
		//class Vertex * corners [NUM_ROOT_VERTICES];
		RootNode root_node;	// Exists purely so that all 'real' nodes have a parent.
		Shader * shader;
	};
}
