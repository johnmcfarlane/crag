//
//  Thruster.cpp
//  crag
//
//  Created by John McFarlane on 11/19/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Thruster.h"

#include "gfx/Scene.h"
#include "gfx/Sphere.h"

#include "glpp/glpp.h"

#include "core/Random.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Thruster member definitions


Thruster::Thruster()
: LeafNode(Layer::foreground)
, _sphere(nullptr)
, _thrust(0)
{
}

void Thruster::Update(UpdateParams const & params)
{
	_thrust = params.thrust;
}

