//
//  Light.h
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

#include "gfx/Color.h"
#include "gfx/LightType.h"

#if ! defined(NDEBUG)
//#define CRAG_GFX_LIGHT_DEBUG
#endif

namespace gfx
{
	// A gfx Object representing a light source.
	class Light : public LeafNode
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		Light(
			Init const & init, Transformation const & local_transformation, 
			Color4f const & color, LightType type, 
			ObjectHandle exception = ObjectHandle());
		~Light();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Light);
		
		bool GetIsExtinguished() const;
		void SetIsExtinguished(bool is_extinguished);
		
		bool GetIsLuminant() const;
		
		void SetColor(Color4f const & color);
		Color4f const & GetColor() const;
		
		virtual void SetAngle(Vector2 params);
		virtual Vector2 GetAngle() const;
		
		LightType GetType() const;
		bool MakesShadow() const;
		
		LeafNode const * GetException() const;
		
#if defined(CRAG_GFX_LIGHT_DEBUG)
		LeafNode::PreRenderResult PreRender() override;
#endif
		
		// variables
	private:
		DEFINE_INTRUSIVE_LIST(Light, List);

		Color4f _color;
		LeafNode const * _exception;
		LightType _type;
		bool _is_extinguished = false;
	};
}
