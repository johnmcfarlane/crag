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
	class VboResource;
	class Program;
	
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
		VboResource & GetVbo(VboIndex index);
		VboResource const & GetVbo(VboIndex index) const;
		
	private:
		bool InitShaders();
		bool InitGeometry();
		
		// shaders and shader programs
		std::array<Program *, std::size_t(ProgramIndex::size)> _programs;
		
		// stock geometry
		std::array<VboResource *, std::size_t(VboIndex::size)> _vbos;
	};
}
