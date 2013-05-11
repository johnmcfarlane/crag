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
		Program * GetProgram(ProgramIndex index);
		Program const * GetProgram(ProgramIndex index) const;
		
		// buffer object functions
		Cuboid const & GetCuboid() const;
		Quad const & GetSphereQuad() const;
		Quad const & GetDiskQuad() const;
		
	private:
		bool InitShaders();
		bool InitGeometry();
		
		// constants
		static constexpr std::size_t _num_programs = std::size_t(ProgramIndex::size);

		// shaders and shader programs
		Program * _programs[_num_programs];
		
		// stock geometry
		Cuboid * _cuboid;
		Quad * _sphere_quad;
		Quad * _disk_quad;
	};
}
