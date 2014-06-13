//
//  gfx/defs.h
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

// forward-declarations
namespace crag
{
	namespace core
	{
		template <typename Type> class ResourceHandle;
	}
}

namespace ipc
{
	class Uid;
}

namespace geom
{
	template <typename S, int D> class Vector;
	template <typename S, int R, int C> class Matrix;
	template <typename S> class Transformation;
	template <typename S, int D> class Ray;
}

namespace gfx
{
	class Program;
	class VboResource;
	template <typename Vertex, typename Index> class Mesh;
	template<typename VERTEX, GLenum USAGE> class IndexedVboResource;

	// render layers
	enum class Layer
	{
		background,
		light,
		opaque,
		transparent
	};
	
	// index buffer object type

#if defined(CRAG_USE_GLES)
	typedef GLushort ElementIndex;
#define CRAG_GFX_ELEMENT_INDEX_TYPE GL_UNSIGNED_SHORT
#else
	typedef GLuint ElementIndex;
#define CRAG_GFX_ELEMENT_INDEX_TYPE GL_UNSIGNED_INT
#endif

	// resource handle types
	using ProgramHandle = crag::core::ResourceHandle<Program>;
	using VboResourceHandle = crag::core::ResourceHandle<VboResource>;
	
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
