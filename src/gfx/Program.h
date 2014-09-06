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
	
	class Program : private crag::counted_object<Program>
	{
		OBJECT_NO_COPY(Program);
	public:
		Program(Program && rhs);
		Program(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		virtual ~Program();
		
		bool IsInitialized() const;

		bool IsLinked() const;
		void Link();

		bool IsBound() const;
		void Bind() const;
		void Unbind() const;
		
		void SetNeedsMatrixUpdate(bool needs_update) const;
		bool NeedsMatrixUpdate() const;
		
		void SetNeedsLightsUpdate(bool needs_update) const;
		bool NeedsLightsUpdate() const;
		
		virtual void SetProjectionMatrix(Matrix44 const & projection_matrix) const;
		virtual void SetModelViewMatrix(Matrix44 const & model_view_matrix) const;
		virtual void SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const;
		
	protected:
		void BindAttribLocation(int index, char const * name) const;
		template <typename Type> void InitUniformLocation(Uniform<Type> & uniform, char const * name) const;

		virtual void InitUniforms();

	private:
		bool DumpInfoLog() const;
		void Verify() const;

		// variables
		GLuint _id;
		Shader _vert_shader;
		Shader _frag_shader;
		mutable bool _needs_matrix_update;
		mutable bool _needs_lights_update;
	};

	template <typename ProgramType>
	ProgramType MakeProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources)
	{
		auto program = ProgramType(vert_sources, frag_sources);
		program.Link();
		return program;
	}
	
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
		
	protected:
		void InitUniforms() override;

		// variables
		Uniform<Matrix44> _projection_matrix;
		Uniform<Matrix44> _model_view_matrix;
	};
		
	// a program that requires light-related information
	class LightProgram : public Program3d
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		template <typename ELEMENT>
		using Array = std::array<std::array<ELEMENT, int(LightResolution::size)>, int(LightResolution::size)>;

	public:
		// set of uniform ids needed to specify lights to a glsl program
		struct LightUniforms
		{
			OBJECT_NO_COPY(LightUniforms);
			LightUniforms() = default;
			LightUniforms(LightUniforms &&);
			LightUniforms & operator=(LightUniforms &&);

			Uniform<Vector3> position;
			Uniform<Vector3> direction;
			Uniform<Color4f> color;
			Uniform<Vector2> angle;
			Uniform<int> type;
		};

		using super = Program3d;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		LightProgram(LightProgram && rhs);
		LightProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
	protected:
		void InitUniforms() override;

		void SetLights(Color4f const & ambient, Light::List const & lights, LightFilter const & filter) const override;

	private:
		
		////////////////////////////////////////////////////////////////////////////////
		// constants
		
		// matches value in assets/glsl/light_commont.glsl
#if defined(CRAG_USE_GLES)
		static constexpr auto max_vertex_lights = 1;
		static constexpr auto max_fragment_lights = 0;
#else
		static constexpr auto max_vertex_lights = 6;
		static constexpr auto max_fragment_lights = 1;
#endif
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// flat light array
		std::array<LightUniforms, max_vertex_lights> _vertex_lights;
		std::array<LightUniforms, max_fragment_lights> _fragment_lights;
	};
	
	// Things in the 3D world but in front of the skybox
	class ForegroundProgram : public LightProgram
	{
		using super = LightProgram;

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

	class PolyProgram final : public ForegroundProgram
	{
		// types
		typedef ForegroundProgram super;
		
		// functions
	public:
		PolyProgram(PolyProgram && rhs);
		PolyProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};

	// for rendering shadow volumes
	class ShadowProgram final : public Program3d
	{
	public:
		ShadowProgram(ShadowProgram && rhs);
		ShadowProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	class ScreenProgram final : public Program
	{
	public:
		ScreenProgram(ScreenProgram && rhs);
		ScreenProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	class DiskProgram final : public ForegroundProgram
	{
	public:
		// types
		typedef ForegroundProgram super;
		
		// functions
		DiskProgram(DiskProgram && rhs);
		DiskProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
		void SetUniforms(geom::Transformation<float> const & model_view, float radius, Color4f const & color) const;
	private:
		void InitUniforms() override;

		// variables
		Uniform<Vector3> _center;
		Uniform<float> _radius;
	};
	
	// TODO: Rename Skybox
	class TexturedProgram final : public LightProgram
	{
	public:
		TexturedProgram(TexturedProgram && rhs);
		TexturedProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
	};
	
	// used to render text
	class SpriteProgram final : public Program
	{
		// types
		typedef Program super;

	public:
		SpriteProgram(SpriteProgram && rhs);
		SpriteProgram(std::initializer_list<char const *> vert_sources, std::initializer_list<char const *> frag_sources);
		
		void SetUniforms(geom::Vector2i const & resolution) const;
	private:
		void InitUniforms() override;
		
		// variables
		Uniform<Vector2> _position_scale;
		Uniform<Vector2> _position_offset;
	};
}
