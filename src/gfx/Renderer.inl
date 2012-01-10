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
	if (scene == nullptr)
	{
		return;
	}

	Object * base_object = scene->GetObject(uid);

	if (base_object == nullptr)
	{
		// Presumably, the object was removed by script thread
		// but a pending update message came in from simulation.
		return;
	}

	OBJECT & derived_object = static_cast<OBJECT &>(* base_object);
	derived_object.Update(params, * this);
}
