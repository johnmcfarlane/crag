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

	// an application-specific shader program that manages its shaders
	// TODO: Make SetUniforms virtual, include lights and matrices, 
	// implement dirty cache to lazily update uniforms when bound
	class Program
	{
		OBJECT_NO_COPY(Program);
	public:
		Program();
		virtual ~Program();
		
		bool IsInitialized() const;
		bool IsLinked() const;
		bool IsBound() const;
		
		bool Init(char const * const * vert_sources, char const * const * frag_sources);
		void Deinit();
		
		void Bind() const;
		void Unbind() const;
		
		void SetProjectionMatrix(Matrix44 const & projection_matrix) const;
		void SetModelViewMatrix(Matrix44 const & model_view_matrix) const;
		
	protected:
		GLint GetUniformLocation(char const * name) const;

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
	};
	
	// a program that requires light-related information
	class LightProgram : public Program
	{
		// set of uniform ids needed to specify lights to a glsl program
		struct LightLocation
		{
			unsigned position = 0;
			unsigned color = 0;
		};

	public:
		virtual void InitUniforms() override;
		void SetLight(Light const & light);
		void SetLights(Light::List const & lights, LightType filter);

	private:

		void SetLight(Light const & light, int index);
		void AddLight();
		
		unsigned _num_lights_location;
		std::vector<LightLocation> _light_locations;
	};

	class PolyProgram : public LightProgram
	{
		// types
		typedef LightProgram super;
		
		// functions
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

	class ShadowProgram : public Program
	{
	public:
		ShadowProgram();
	private:
		virtual void InitAttribs(GLuint id) final;
	};
	
	class ScreenProgram : public Program
	{
	public:
		ScreenProgram();
	private:
		virtual void InitAttribs(GLuint id) final;
	};
	
	class DiskProgram : public LightProgram
	{
		// types
		typedef LightProgram super;
		
		// functions
	public:
		DiskProgram();
		
		void SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const;
	private:
		virtual void InitAttribs(GLuint id) override;
		virtual void InitUniforms() override;

		// variables
		GLint _color_location;
		GLint _center_location;
		GLint _radius_location;
	};
	
	class FogProgram : public DiskProgram
	{
	public:
		FogProgram();
		
		void SetUniforms(geom::Transformation<float> const & model_view, Color4f const & color, float radius, float density) const;
	private:
		virtual void InitAttribs(GLuint id) override final;
		virtual void InitUniforms() override final;
		
		// variables
		GLint _density_location;
	};
	
	// used by skybox
	class TexturedProgram : public Program
	{
	private:
		virtual void InitAttribs(GLuint id) override final;
	};
	
	// used to render text
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
