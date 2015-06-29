//
//  sim/Model.h
//  crag
//
//  Created by john on 2015-06-25.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include <pch.h>

#include "Model.h"
#include "Engine.h"

#include <physics/Location.h>

#include <gfx/Engine.h>
#include <gfx/object/Object.h>

#include <core/RosterObjectDefine.h>

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Model member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	Model,
	100,
	Pool::Call<& Model::Update>(Engine::GetDrawRoster()));

Model::Model(Handle handle, physics::Location const & location)
	: _handle(handle)
	, _location(location)
{
	CRAG_VERIFY_TRUE(_handle.IsInitialized());
	CRAG_VERIFY_REF(_location);
}

Model::~Model()
{
	_handle.Release();
}

Model::Handle Model::GetHandle() const
{
	return _handle;
}

void Model::Update()
{
	CRAG_VERIFY_TRUE(_handle.IsInitialized());

	auto transformation = _location.GetTransformation();

	_handle.Call([transformation] (gfx::Object & node) {
		node.SetLocalTransformation(transformation);
	});
}

