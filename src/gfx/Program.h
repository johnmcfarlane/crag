//
//  Program.h
//  crag
//
//  Created by John McFarlane on 2011-01-09.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Light.h"

#include "glpp/Shader.h"


namespace gfx
{
	// function declarations
	bool InitShader(gl::Shader & shader, char const * filename, GLenum shader_type);

	// GPUs were invented by people who can't bare to see macros phased out.
	enum
	{
		MAX_LIGHTS = 7
	};
	
	// set of uniform ids needed to specify lights to a glsl program
	struct LightBlock
	{
		struct Light
		{
			unsigned position;
			unsigned color;
		};
		
		Light lights [MAX_LIGHTS];
	};
	
	// an application-specific shader program that manages its shaders
	class Program : public gl::Program
	{
	public:
		void Init(char const * vert_source, char const * frag_source, gl::Shader & light_shader);
		void Deinit(gl::Shader & light_shader);
		
		void Use() const;
		void Disuse() const;
		
		void UpdateLights(Light::List const & lights) const;
		
	private:
		gl::Shader _vert_shader;
		gl::Shader _frag_shader;
		LightBlock _light_block;
	};
}
