/*
 *  GenerateMeshFunctor.cpp
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "GenerateMeshFunctor.h"
#include "ForEachNodeFace.h"

#include "Mesh.h"


void form::GenerateMeshFunctor::GenerateMesh(Node & node)
{
	ForEachNodeFace(node, mesh);
}
