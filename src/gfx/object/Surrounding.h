//
//  gfx/object/Surrounding.h
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"
#include "FormationGeneration.h"

#include "core/double_buffer.h"

namespace gfx
{
	// the graphical representation of form::Surrounding
	class Surrounding final : public LeafNode
	{
		// types
		typedef LeafNode super;
		
	public:
		// functions
		Surrounding(Init const & init, int max_num_quaterne);
		~Surrounding();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Surrounding);
		
		virtual void UpdateModelViewTransformation(Transformation const & model_view) override;

		void SetMesh(std::shared_ptr<form::Mesh> const & mesh);
		
	private:
		PreRenderResult PreRender() override;
		void GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const override;
		void Render(Engine const & renderer) const override;
		
		void OnMeshResourceChange();
		void ReturnMesh(std::shared_ptr<form::Mesh> const & mesh);
		
		Vector3 GfxToForm(Vector3 const & position) const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// CPU-side mesh geometry sent from form::Engine
		std::shared_ptr<form::Mesh> _mesh;
		
		// contains the geometry and GL state
		FormationGeneration _generation;

		int const _max_num_quaterne;
	};
}

