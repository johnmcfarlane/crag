//
//  gfx/object/Surrounding.h
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"

#include "Object.h"

#include "form/Mesh.h"

namespace form
{
	class Mesh;
}

namespace gfx
{
	struct LitVertex;
	
	template<typename VERTEX, GLenum USAGE> 
	class NonIndexedVboResource;

	template<typename VERTEX, GLenum USAGE> 
	class IndexedVboResource;

	// the graphical representation of form::Surrounding
	class Surrounding final : public Object
	{
		// types
#if defined(CRAG_FORM_FLAT_SHADE)
		using VboResource = gfx::NonIndexedVboResource<LitVertex, GL_DYNAMIC_DRAW>;
#else
		using VboResource = gfx::IndexedVboResource<LitVertex, GL_DYNAMIC_DRAW>;
#endif
		
	public:
		// functions
		Surrounding(Engine & engine);
		~Surrounding();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Surrounding);
		
		virtual void UpdateModelViewTransformation(Transformation const & model_view) override;

		void SetMesh(std::shared_ptr<form::Mesh> const & mesh);
		
	private:
		PreRenderResult PreRender() override;
		bool GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const override;
		void Render(Engine const & renderer) const override;
		
		void OnMeshResourceChange();
		void ReturnMesh(std::shared_ptr<form::Mesh> const & mesh);
		
		Vector3 GfxToForm(Vector3 const & position) const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// CPU-side mesh geometry sent from form::Engine
		std::shared_ptr<form::Mesh> _mesh;
		
		// basically, where is our origin
		form::MeshProperties _properties;
	};
}

