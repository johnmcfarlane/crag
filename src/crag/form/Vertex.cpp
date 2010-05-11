/*
 *  Vertex.cpp
 *  Crag
 *
 *  Created by john on 5/1/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/Vertex.h"


form::Vertex::Vertex() 
: pos(Vector3f::Zero())
, red(0)
, green(0)
, blue(0)
, flags(0)
, norm(Vector3f::Zero())
//	, tex(Vector2f::Zero())
#if VERTEX_INDEX
, _index(Universe::vertices.GetIndex(* this))
#endif
{ 
}
