//
//  icosahedron.h
//  crag
//
//  Created by John McFarlane on 7/7/11.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once


namespace icosahedron
{
	////////////////////////////////////////////////////////////////////////////////
	// Mesh information for an icosahedron 
	
	// no prizes for guessing where I cribbed this code...
	float const x = .525731112119133606;
	float const z = .850650808352039932;
	float const o = 0;
	
	// coordinates of the 12 vertices
	float const verts[][3] = 
	{
		{ -x,  o,  z },
		{  x,  o,  z },
		{ -x,  o, -z },
		{  x,  o, -z },
		{  o,  z,  x },
		{  o,  z, -x },
		{  o, -z,  x },
		{  o, -z, -x },
		{  z,  x,  o },
		{ -z,  x,  o },
		{  z, -x,  o },
		{ -z, -x,  o }
	};
	
	unsigned const num_verts = ARRAY_SIZE(verts);
	
	// indices of the 20 faces
	unsigned const faces[][3] = 
	{ 
		{0,4,1}, 
		{0,9,4}, 
		{9,5,4}, 
		{4,5,8}, 
		{4,8,1},    
		{8,10,1}, 
		{8,3,10}, 
		{5,3,8}, 
		{5,2,3}, 
		{2,7,3},    
		{7,10,3}, 
		{7,6,10}, 
		{7,11,6}, 
		{11,0,6}, 
		{0,1,6}, 
		{6,1,10}, 
		{9,0,11}, 
		{9,11,2}, 
		{9,2,5},
		{7,2,11} 
	};
	
	unsigned const num_faces = ARRAY_SIZE(faces);	
}
