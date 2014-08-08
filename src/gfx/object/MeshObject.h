//
//  gfx/object/MeshObject.h
//  crag
//
//  Created by John McFarlane on 2014-01-15.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"

#include "gfx/LitVertex.h"
#include "gfx/ShadowVolume.h"

#include "ipc/Listener.h"

#include "core/ResourceHandle.h"
#include "core/HashString.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(MeshObject); // gfx::MeshObjectHandle

	// MeshObject class - general purpose mesh object
	class MeshObject final : public Object
	{
	public:
		// functions
		DECLARE_ALLOCATOR(MeshObject);

		MeshObject(Engine & engine, Transformation const & local_transformation, Color4f const & color, Vector3 const & scale, ResourceKey lit_vbo_key, ResourceKey plain_mesh_key);
		
		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;
		
	private:
		void UpdateModelViewTransformation(Transformation const & model_view) final;
		bool GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const final;
		
		// variables
		Color4f _color;
		Vector3 _scale;
		Scalar _bounding_radius;
		PlainMeshHandle _plain_mesh;
		ResourceKey _lit_vbo_key;
		VboResourceHandle _lit_vbo;
	};
}
