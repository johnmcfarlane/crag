//
//  gfx/object/MeshObject.h
//  crag
//
//  Created by John McFarlane on 2014-01-15.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

#include "gfx/LitVertex.h"

#include "core/Resource.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(MeshObject); // gfx::MeshObjectHandle

	// MeshObject class - general purpose mesh object
	class MeshObject final : public LeafNode
	{
	public:
		// functions
		DECLARE_ALLOCATOR(MeshObject);

		MeshObject(Init const & init, Transformation const & local_transformation, Color4f const & color, Vector3 const & scale, LitVboHandle const & lit_vbo, PlainMeshHandle const & plain_mesh);
		~MeshObject();
		
		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;
		
	private:
		void UpdateModelViewTransformation(Transformation const & model_view) final;
		void GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const final;

		// variables
		Color4f _color;
		Vector3 _scale;
		Scalar _bounding_radius;
		PlainMeshHandle _plain_mesh;
		LitVboHandle _lit_vbo;
	};
}
