//
//  Program.h
//  crag
//
//  Created by John McFarlane on 2011-01-09.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Shader.h"
#include "Uniform.h"

#include "gfx/object/Light.h"

#include "geom/Transformation.h"


namespace gfx
{
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
		
		virtual void SetProjectionMatrix(Matrix44 const & projection_matrix) const;
		virtual void SetModelViewMatrix(Matrix44 const & model_view_matrix) const;
		
	protected:
		virtual void InitAttribs(GLuint id);
		virtual void InitUniforms();
		
		void GetInfoLog(std::string & info_log) const;
		void Verify() const;

		// variables
		GLuint _id;
		Shader _vert_shader;
		Shader _frag_shader;
	};
	
	class Program3d : public Program
	{
	public:
		// types
		using super = Program;
		
		// functions
		void SetProjectionMatrix(Matrix44 const & projection_matrix) const final;
		void SetModelViewMatrix(Matrix44 const & model_view_matrix) const final;
		
		void InitUniforms() override;

	private:
		// variables
		Uniform<Matrix44> _projection_matrix;
		Uniform<Matrix44> _model_view_matrix;
	};
		
	// a program that requires light-related information
	class LightProgram : public Program3d
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// set of uniform ids needed to specify lights to a glsl program
		struct LightUniforms
		{
			Uniform<Vector3> position;
			Uniform<Color4f> color;
		};

	public:
		using super = Program3d;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		virtual void InitUniforms() override;
		void SetLight(Light const & light);
		void SetLights(Color4f const & ambient, Light::List const & lights, LightType filter);

	private:

		void SetLight(Light const & light, int index);
		void AddLight();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Uniform<Color4f> _ambient;
		Uniform<int> _num_lights;
		std::vector<LightUniforms> _lights;
	};

	class PolyProgram : public LightProgram
	{
		// types
		typedef LightProgram super;
		
		// functions
	public:
		PolyProgram();
		
		void SetUniforms(Color4f const & color, bool fragment_lighting, bool flat_shade, bool relief_enabled = false) const;
	private:
		virtual void InitAttribs(GLuint id) override;
		virtual void InitUniforms() override final;
		
		// variables
		Uniform<Color4f> _color;
		Uniform<bool> _fragment_lighting;
		Uniform<bool> _flat_shade;
		Uniform<bool> _relief_enabled;
	};

	class ShadowProgram : public Program3d
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
	public:
		// types
		typedef LightProgram super;
		
		// functions
		DiskProgram();
		
		void SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const;
	private:
		virtual void InitAttribs(GLuint id) override;
		virtual void InitUniforms() override;

		// variables
		Uniform<Color4f> _color;
		Uniform<Vector3> _center;
		Uniform<float> _radius;
	};
	
	// used by skybox
	class TexturedProgram : public Program3d
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
		Uniform<Vector2> _position_scale;
		Uniform<Vector2> _position_offset;
	};
}
