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
			ObjectHandle exception = ObjectHandle(), Scalar angle = PI * .25f);
		~Light();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Light);
		
		bool GetIsExtinguished() const;
		void SetIsExtinguished(bool is_extinguished);
		
		bool GetIsLuminant() const;
		
		void SetColor(Color4f const & color);
		Color4f const & GetColor() const;
		
		// TODO: BeamLight derived class with separate render pass
		// after other lights
		void SetAngle(Scalar width);
		Vector2 GetAngleVector() const;
		
		LightType GetType() const;
		bool MakesShadow() const;
		
		LeafNode const * GetException() const;
		
#if defined(CRAG_GFX_LIGHT_DEBUG)
		LeafNode::PreRenderResult PreRender() override;
#endif
		
		// variables
	private:
		// This is the list which is sorted in order of 
		DEFINE_INTRUSIVE_LIST(Light, List);

		Color4f _color;
		Vector2 _angle;	// search light sin/cos
		LeafNode const * _exception;
		LightType _type;
		bool _is_extinguished = false;
	};
}
