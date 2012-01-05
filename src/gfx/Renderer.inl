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

	// Need to include Scene.h wherever this is referred to.
	ObjectMap & objects = scene->GetObjectMap();
	ObjectMap::iterator i = objects.find(uid);
	if (i == objects.end())
	{
		// Presumably, the object was removed by script thread
		// but a pending update message came in from simulation.
		return;
	}

	OBJECT & object = static_cast<OBJECT &>(* i->second);
	object.Update(params, * this);
}
