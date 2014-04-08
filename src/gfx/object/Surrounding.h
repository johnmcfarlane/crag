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

#if defined(CRAG_FLAT_SHADE)
#include "gfx/NonIndexedVboResource.h"
#else
#include "gfx/IndexedVboResource.h"
#endif

#include "LeafNode.h"

#include "gfx/LitVertex.h"

#include "form/Mesh.h"

namespace gfx
{
	// the graphical representation of form::Surrounding
	class Surrounding final : public LeafNode
	{
		// types
		typedef LeafNode super;
#if defined(CRAG_FLAT_SHADE)
		using VboResource = gfx::NonIndexedVboResource<LitVertex, GL_DYNAMIC_DRAW>;
#else
		using VboResource = gfx::IndexedVboResource<LitVertex, GL_DYNAMIC_DRAW>;
#endif
		
	public:
		// functions
		Surrounding(Init const & init);
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
		
		// contains the geometry and GL state
		VboResource _vbo_resource;
	};
}

