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

#include "sim/defs.h"


namespace gfx
{
	// forward-declarations
	struct LightInfo;
	
	// A gfx Object representing a light source.
	class Light : public LeafNode
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		struct UpdateParams
		{
			Vector3 position;
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		Light(Init const & init, Transformation const & local_transformation, Color4f const & color);
		~Light();
		
#if defined(VERIFY)
		virtual void Verify() const override;
#endif
		
		void SetColor(Color4f const & color);
		Color4f const & GetColor() const;
		
#if ! defined(NDEBUG)
		LeafNode::PreRenderResult PreRender() override;
#endif
		
		// variables
	private:
		// This is the list which is sorted in order of 
		DEFINE_INTRUSIVE_LIST(Light, List);

		Color4f _color;
	};
}
