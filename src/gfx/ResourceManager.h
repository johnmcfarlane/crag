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
	class Model;
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
		
		// model functions
		Model & GetModel(ModelIndex index);
		Model const & GetModel(ModelIndex index) const;
		
		// program functions
		Program * GetProgram(ProgramIndex index);
		Program const * GetProgram(ProgramIndex index) const;
		
		// vertex buffer object functions
		VboResource & GetVbo(VboIndex index);
		VboResource const & GetVbo(VboIndex index) const;
		
	private:
		bool InitModels();
		bool InitShaders();
		bool InitVbos();
		
		// stock geometry
		std::array<Model *, std::size_t(ModelIndex::size)> _models;
		
		// shaders and shader programs
		std::array<Program *, std::size_t(ProgramIndex::size)> _programs;
		
		// GPU geometry buffers
		std::array<VboResource *, std::size_t(VboIndex::size)> _vbos;
	};
}
