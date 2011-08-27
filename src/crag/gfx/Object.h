//
//  Object.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "geom/Ray.h"
#include "geom/Matrix4.h"


namespace gfx
{
	// forward-declarations
	class Scene;
	
	// Base class for drawable things.
	class Object
	{
	public:
		// types
		typedef double Scalar;
		typedef Vector<Scalar, 3> Vector;
		typedef Matrix4<Scalar> Matrix;
		typedef Ray<Scalar, 3> Ray;
		
		// functions
		Object(Vector const & position = Vector::Zero(), Matrix const & rotation = Matrix::Identity());
		virtual ~Object();
		
		virtual void Init();	// called on arrival in render thread 
		
		Vector3d const & GetPosition() const;
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		
		// at states for which IsInLayer returns true
		virtual void Render(Layer::type layer, Scene const & scene) const = 0;
		
		// returns true iff this object belongs in the given render layer;
		// currently must remain invariant
		virtual bool IsInLayer(Layer::type) const = 0;
		
		// variables
		Vector _position;
		Matrix _rotation;
	};
	
	// messages
	struct AddObjectMessage
	{
		Object & _object;
	};
	
	struct RemoveObjectMessage
	{
		Object & _object;
	};
	
	template <typename OBJECT>
	struct UpdateObjectMessage
	{
		UpdateObjectMessage(OBJECT & object) 
		: _object(object)
		, _updated(object) 
		{ 
		}
		
		OBJECT & _object;
		OBJECT _updated;
	};
}
