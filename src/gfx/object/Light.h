/*
 *  gfx/object/Light.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "LeafNode.h"

#include "gfx/Color.h"

#include "sim/defs.h"

#include "glpp/glpp.h"


namespace gfx
{
	class Light : public LeafNode
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// map of the available 8 OpenGL light slots
		typedef uint8_t SlotMap;
	public:
		struct UpdateParams
		{
			Vector3 position;
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		Light(Color4f const & col, float a = 0, float b = 0, float c = 1, bool init_shadows = false);
		
		void Init(Scene const & scene) override;
		void Deinit() override;
		
		virtual void Render() const;
		
		// variables
	private:
		Color4f color;
		float attenuation_a;
		float attenuation_b;
		float attenuation_c;
		GLenum light_id;
		
		static SlotMap _used_slots;
	};
}
