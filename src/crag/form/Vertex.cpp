/*
 *  Vertex.cpp
 *  Crag
 *
 *  Created by john on 5/1/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
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
