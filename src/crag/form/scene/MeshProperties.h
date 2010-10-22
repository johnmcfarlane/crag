/*
 *  MeshProperties.h
 *  crag
 *
 *  Created by John on 10/21/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace form 
{
	
	// Common info shared between Mesh and MeshBufferObject.
	struct MeshProperties
	{
		typedef Vector3d Vector3;
		
		MeshProperties(bool init_flat_shaded = false, Vector3 const & init_origin = Vector3::Zero()) 
		: origin(init_origin)
		, flat_shaded(init_flat_shaded)
		{
		}
		
		Vector3 origin;
		bool flat_shaded;
	};
	
}
