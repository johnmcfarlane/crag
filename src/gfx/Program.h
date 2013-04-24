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

#include "Shader.h"

#include "geom/Transformation.h"


namespace gfx
{
	// function declarations
	GLuint InitShader(char const * filename, GLenum shader_type);

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
	class Program
	{
	public:
		Program();
		virtual ~Program();
		
		bool IsInitialized() const;
		bool IsLinked() const;
		bool IsBound() const;
		
		virtual void Init(char const * vert_source, char const * frag_source, Shader & light_vert_shader, Shader & light_frag_shader);
		void Deinit(Shader & light_vert_shader, Shader & light_frag_shader);
		
		void Bind() const;
		void Unbind() const;
		
		// refers to the frame-invariant uniforms
		void SetUniformsValid(bool uniforms_invalid);
		bool GetUniformsValid() const;
		
		void UpdateLights(Light::List const & lights) const;
		void SetProjectionMatrix(Matrix44 const & projection_matrix) const;
		void SetModelViewMatrix(Matrix44 const & model_view_matrix) const;
		
	protected:
		GLint GetUniformLocation(char const * name) const;

	private:
		virtual void InitAttribs(GLuint id);
		virtual void InitUniforms();
		
		void GetInfoLog(std::string & info_log) const;
		void Verify() const;

		// variables
		GLuint _id;
		Shader _vert_shader;
		Shader _frag_shader;
		GLint _projection_matrix_location;
		GLint _model_view_matrix_location;
		LightBlock _light_block;
		bool _uniforms_valid;
	};

	class PolyProgram : public Program
	{
	public:
		PolyProgram();
		
		void SetUniforms(Color4f const & color, bool fragment_lighting, bool flat_shade) const;
	private:
		virtual void InitAttribs(GLuint id) override;
		virtual void InitUniforms() override final;
		
		// variables
		GLint _color_location;
		GLint _fragment_lighting_location;
		GLint _flat_shade_location;
	};
	
	class DiskProgram : public Program
	{
	public:
		DiskProgram();
		
		void SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color) const;
	private:
		virtual void InitAttribs(GLuint id) override;
		virtual void InitUniforms() override;
		static Scalar CalculateRadius(geom::Transformation<float> const & transformation);

		// variables
		GLint _color_location;
		GLint _center_location;
		GLint _radius_location;
	};
	
	class FogProgram : public DiskProgram
	{
	public:
		FogProgram();
		
		void SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color, float density) const;
	private:
		virtual void InitAttribs(GLuint id) override final;
		virtual void InitUniforms() override final;
		
		// variables
		GLint _density_location;
	};
	
	class TexturedProgram : public Program
	{
	private:
		virtual void InitAttribs(GLuint id) override final;
	};
	
	class SpriteProgram : public Program
	{
	public:
		void SetUniforms(geom::Vector2i const & resolution) const;
	private:
		virtual void InitAttribs(GLuint id) override final;
		virtual void InitUniforms() override final;
		
		// variables
		GLint _position_scale_location;
		GLint _position_offset_location;
	};
}
