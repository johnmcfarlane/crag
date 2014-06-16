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
	// Class Hierarchy:
	//
	//	Program:
	//		ScreenProgram
	//		SpriteProgram
	//		Program3d:
	//			ShadowProgram
	//			LightProgram:
	//				TexturedProgram
	//				ForegroundProgram:
	//					PolyProgram
	//					DiskProgram
	
	// an application-specific shader program that manages its shaders
	// TODO: Make SetUniforms virtual, include lights and matrices, 
	// implement dirty cache to lazily update uniforms when bound
	
	class Program
	{
		OBJECT_NO_COPY(Program);
	public:
		Program(Program && rhs);
		Program(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		virtual ~Program();
		
		bool IsInitialized() const;
		bool IsLinked() const;
		bool IsBound() const;
		
		void Bind() const;
		void Unbind() const;
		
		virtual void SetProjectionMatrix(Matrix44 const & projection_matrix) const;
		virtual void SetModelViewMatrix(Matrix44 const & model_view_matrix) const;
		
	protected:
		void BindAttribLocation(int index, char const * name) const;
		template <typename Type> void InitUniformLocation(Uniform<Type> & uniform, char const * name) const;
		void Finalize();	// must be called at end of construction (hacky)

	private:
		virtual void InitUniforms();
		bool DumpInfoLog() const;
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
		Program3d(Program3d && rhs);
		Program3d(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
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
			Uniform<Vector3> direction;
			Uniform<Color4f> color;
			Uniform<Vector2> angle;
		};
		
		template <typename ELEMENT>
		using Array = std::array<std::array<ELEMENT, std::size_t(LightType::size)>, std::size_t(LightResolution::size)>;

	public:
		using super = Program3d;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		virtual void InitUniforms() override;
		LightProgram(LightProgram && rhs);
		LightProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
		virtual void SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const;

	private:
		
		////////////////////////////////////////////////////////////////////////////////
		// constants
		
		// matches value in assets/glsl/light.glsl
		static constexpr auto max_attribute_lights = 6;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Array<Uniform<int>> _num_lights;
		Array<std::array<LightUniforms, max_attribute_lights>> _lights;
	};
	
	// Things in the 3D world but in front of the skybox
	class ForegroundProgram : public LightProgram
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		ForegroundProgram(ForegroundProgram && rhs);
		ForegroundProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);

		void SetUniforms(Color4f const & color) const;
	protected:
		void InitUniforms() override;
	private:
		void SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const final;

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Uniform<Color4f> _ambient;
		Uniform<Color4f> _color;
	};

	class PolyProgram : public ForegroundProgram
	{
		// types
		typedef ForegroundProgram super;
		
		// functions
	public:
		PolyProgram(PolyProgram && rhs);
		PolyProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};

	// for rendering shadow volumes
	class ShadowProgram : public Program3d
	{
	public:
		ShadowProgram(ShadowProgram && rhs);
		ShadowProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	class ScreenProgram : public Program
	{
	public:
		ScreenProgram(ScreenProgram && rhs);
		ScreenProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	class DiskProgram : public ForegroundProgram
	{
	public:
		// types
		typedef ForegroundProgram super;
		
		// functions
		DiskProgram(DiskProgram && rhs);
		DiskProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
		void SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const;
	private:
		virtual void InitUniforms() override;

		// variables
		Uniform<Vector3> _center;
		Uniform<float> _radius;
	};
	
	// TODO: Rename Skybox
	class TexturedProgram : public LightProgram
	{
	public:
		TexturedProgram(TexturedProgram && rhs);
		TexturedProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	// used to render text
	class SpriteProgram : public Program
	{
	public:
		SpriteProgram(SpriteProgram && rhs);
		SpriteProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
		void SetUniforms(geom::Vector2i const & resolution) const;
	private:
		virtual void InitUniforms() override final;
		
		// variables
		Uniform<Vector2> _position_scale;
		Uniform<Vector2> _position_offset;
	};
}
