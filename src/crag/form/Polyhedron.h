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
	class PointBuffer;
	
	class Polyhedron
	{
	public:
		Polyhedron();
		~Polyhedron();
		
		void InitPoints(PointBuffer & points);
		void DeinitPoints(PointBuffer & points);

		void SetShader(class Shader * init_shader);
		Shader & GetShader();
		
		enum 
		{
			NUM_ROOT_POINTS = 4
		};
		
		class Point * corners [NUM_ROOT_POINTS];
		Node root_node;
		
	private:
		Shader * shader;
	};
}
