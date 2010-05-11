/*
 *  Model.cpp
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "Model.h"
#include "Shader.h"
#include "VertexBuffer.h"

#include "core/memory.h"


/////////////////////////////////////////////////////////////////
// form::Model

form::Model::Model()
: root_node(* this)
, shader(nullptr)
{
}

form::Model::Model(Model const & rhs)
: root_node(* this)
, shader(nullptr)
{
	// Can only copy default-state objects.
	Assert(rhs.shader == nullptr);
}

form::Model::~Model()
{
	delete shader;
}

void form::Model::SetShader(form::Shader * init_shader)
{
	// Currently, no need ever to change shader.
	Assert(shader == nullptr);
	
	shader = init_shader;
}

form::Shader & form::Model::GetShader()
{
	Assert(shader != nullptr);
	return * shader;
}

