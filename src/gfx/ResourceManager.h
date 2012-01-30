//
//  ResourceManager.h
//  crag
//
//  Created by John McFarlane on 2012-01-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"


namespace gfx
{
	// forward-declarations
	class Cuboid;
	class Program;
	class Quad;
	class Shader;
	
	// Central store for OpenGL resources which are:
	// 1. shared between multiple objects and
	// 2. require expensive state changes.
	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();
		
		// program functions
		Program const * GetProgram(ProgramIndex::type index) const;
		
		// buffer object functions
		Cuboid const & GetCuboid() const;
		Quad const & GetSphereQuad() const;
		Quad const & GetDiskQuad() const;
		
	private:
		bool InitShaders();
		bool InitGeometry();

		// shaders and shader programs
		Shader * _light_vert_shader;
		Shader * _light_frag_shader;
		Program * _programs[ProgramIndex::max_index];
		
		// stock geometry
		Cuboid * _cuboid;
		Quad * _sphere_quad;
		Quad * _disk_quad;
	};
}
