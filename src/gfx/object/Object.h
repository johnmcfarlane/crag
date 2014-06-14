//
//  gfx/object/Object.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ipc/Object.h"

#include "gfx/defs.h"

#include "geom/Transformation.h"

namespace gfx { DECLARE_CLASS_HANDLE(Object); }	// gfx::ObjectHandle


namespace gfx
{
	// forward-declarations
	class Pov;
	class Engine;
	class Light;
	class Scene;

	// ShadowVolume forward-declaration
	template <typename VERTEX, GLenum USAGE> class NonIndexedVboResource;
	struct PlainVertex;
	using ShadowVolume = gfx::NonIndexedVboResource<PlainVertex, GL_DYNAMIC_DRAW>;
	
	// function declarations
	bool IsChild(Object const & child, Object const & parent);
	void AdoptChild(Object & child, Object & parent);
	void OrphanChild(Object & child, Object & parent);
	void OrphanChild(Object & child);

	bool operator < (Object const &, Object const &);

	// Base class for drawable things.
	// meaning they effectively double as nodes in a hierachical scene graph.
	class Object : public ipc::Object<Object, Engine>
	{
	protected:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef ipc::Object<Object, Engine> super;
		
	public:
		enum PreRenderResult
		{
			ok,
			remove
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Object(Init const & init, Transformation const & local_transformation, Layer layer, bool casts_shadow = false);
		virtual ~Object();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Object);
		
		//////////////////////////////////////////////////////////////////////////////
		// tree structure

	private:
		Object * _parent;
		DEFINE_INTRUSIVE_LIST(Object, List);
		List _children;

	public:
		friend bool IsChild(Object const & child, Object const & parent);
		friend void AdoptChild(Object & child, Object & parent);
		friend void OrphanChild(Object & child, Object & parent);
		friend void OrphanChild(Object & child);
		
		Object * GetParent();
		Object const * GetParent() const;
		
		List & GetChildren();
		List const & GetChildren() const;

		//////////////////////////////////////////////////////////////////////////////
		// transformation

		Transformation const & GetLocalTransformation() const;
		void SetLocalTransformation(Transformation const & local_transformation);
		
		Transformation GetModelTransformation() const;

		virtual void UpdateModelViewTransformation(Transformation const & model_view_transformation);
		void SetModelViewTransformation(Transformation const & model_view_transformation);
		Transformation const & GetModelViewTransformation() const;
		virtual Transformation const & GetShadowModelViewTransformation() const;
		
		friend bool operator < (Object const & lhs, Object const & rhs);
		
		Layer GetLayer() const;
		
		Program const * GetProgram() const;
		void SetProgram(Program const * program);
		
		VboResource const * GetVboResource() const;
		void SetVboResource(VboResource const * mesh_resource);
		
		bool CastsShadow() const;
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(RenderRange & range) const;
		
		// Perform any necessary preparation for rendering.
		virtual PreRenderResult PreRender();
		
		// returns false iff light is obscured by object and should be ignored
		virtual bool GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const;
		
		// Draw the object.
		virtual void Render(Engine const & renderer) const;
		
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		Transformation _local_transformation;
		Transformation _model_view_transformation;
		
		// This is the list which is sorted in render order.
		DEFINE_INTRUSIVE_LIST(Object, RenderList);
		
		float _render_depth;
		Layer const _layer;
		Program const * _program;
		VboResource const * _vbo_resource;
		bool const _casts_shadow;
	};
}
