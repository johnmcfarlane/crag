/*
 *  Image.h
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Node.h"


namespace form
{
	class VertexBuffer;
	
	class Image
	{
	public:
		Image();
		~Image();
		
		void InitVertices(VertexBuffer & vertices);
		void DeinitVertices(VertexBuffer & vertices);

		void SetShader(class Shader * init_shader);
		Shader & GetShader();
		
		enum 
		{
			NUM_ROOT_VERTICES = 4
		};
		
		class Vertex * corners [NUM_ROOT_VERTICES];
		Node root_node;
		
	private:
		Shader * shader;
	};
}
