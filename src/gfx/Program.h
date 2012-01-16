//
//  Program.h
//  crag
//
//  Created by John McFarlane on 2011-01-09.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Light.h"

#include "glpp/Shader.h"

#include "geom/Transformation.h"


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
		virtual void Init(char const * vert_source, char const * frag_source, gl::Shader & light_shader);
		void Deinit(gl::Shader & light_shader);
		
		void Use() const;
		void Disuse() const;
		
		void UpdateLights(Light::List const & lights) const;
		
	private:
		virtual void InitUniforms();
		
		gl::Shader _vert_shader;
		gl::Shader _frag_shader;
		LightBlock _light_block;
	};
	
	class SphereProgram : public Program
	{
	public:
		SphereProgram();
		
		void SetUniforms(::Transformation<float> const & transformation, Color4f const & color) const;
	private:
		void InitUniforms() override;
		static Scalar CalculateRadius(::Transformation<float> const & transformation);

		// variables
		GLint _color_location;
		GLint _center_location;
		GLint _radius_location;
	};
	
	class FogProgram : public SphereProgram
	{
	public:
		FogProgram();
		
		void SetUniforms(::Transformation<float> const & transformation, Color4f const & color, float density) const;
	private:
		void InitUniforms() override;
		
		// variables
		GLint _density_location;
	};
	
	class DiskProgram : public SphereProgram
	{
	public:
	};
}
