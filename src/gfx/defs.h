//
//  defs.h
//  crag
//
//  Created by John McFarlane on 7/18/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if ! defined(CRAG_USE_GLES) && (defined(PROFILE) || ! defined(NDEBUG))
#define CRAG_GFX_DEBUG
#endif

namespace geom
{
	// forward-declarations
	template <typename S, int D> class Vector;
	template <typename S, int R, int C> class Matrix;
	template <typename S> class Transformation;
	template <typename S, int D> class Ray;
}

namespace gfx
{
	enum class ModelIndex
	{
		cuboid,
		lit_cuboid,
		size
	};
	
	enum class ProgramIndex
	{
		poly,	// position, normal and color
		shadow,	// void; used to write to stencil buffer
		screen,	// [-1..1],[-1..1] maps to screen corners
		sphere,	// quad with offset Z
		fog,	// TODO
		disk,	// quad with original Z
		skybox,	// flat shaded with texture UVs
		sprite,	// screen space with texture UVs
		dont_care,	// don't bother setting a different program for this render object
		size = dont_care
	};
	
	enum class VboIndex
	{
		cuboid,
		sphere_quad,
		disk_quad,
		size
	};
	
	// render layers
	enum class Layer
	{
		background,
		light,
		foreground
	};
	
	enum class LightType
	{
		simple,
		shadow,
		size,
		all
	};
	
	// index buffer object type

#if defined(CRAG_USE_GLES)
	typedef GLushort ElementIndex;
#define CRAG_GFX_ELEMENT_INDEX_TYPE GL_UNSIGNED_SHORT
#else
	typedef GLuint ElementIndex;
#define CRAG_GFX_ELEMENT_INDEX_TYPE GL_UNSIGNED_INT
#endif

	// geometric types
	typedef float Scalar;
	typedef geom::Vector<Scalar, 2> Vector2;
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Vector<Scalar, 4> Vector4;
	typedef geom::Vector<Scalar, 2> RenderRange;	// near/far z
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
	typedef geom::Transformation<Scalar> Transformation;
	typedef geom::Triangle<Scalar, 3> Triangle3;
	typedef geom::Ray<Scalar, 3> Ray3;
	
	// unique identifier for gfx::Object;
	// used when refering to objects from outside the render thread
	typedef ipc::Uid Uid;
	
	// base class of objects which are rendered
	class Object;
}
