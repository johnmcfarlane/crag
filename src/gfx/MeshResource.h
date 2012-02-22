//
//  MeshResource.h
//  crag
//
//  Created by John McFarlane on 2012-01-25.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2012 John McFarlane. All rights reserved.
//

#pragma once


namespace gfx
{
	// Base class for a graphical resource such as a VBO / Mesh
	// which is costly to bind / unbind during rendering.
	class MeshResource
	{
	public:
		virtual ~MeshResource() { }
		virtual void Activate() const { }
		virtual void Deactivate() const { }
	};
}
