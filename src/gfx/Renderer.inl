//
//  Renderer.inl
//  crag
//
//  Created by John on 2011-11-25.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Renderer.h"
#include "Scene.h"

template <typename OBJECT> 
void gfx::Renderer::OnUpdateObject(Uid const & uid, typename OBJECT::UpdateParams const & params)
{
	if (scene != nullptr)
	{
		// Need to include Scene.h wherever this is referred to.
		scene->UpdateObject<OBJECT>(uid, params);
	}
}
