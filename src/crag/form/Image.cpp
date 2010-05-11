/*
 *  Image.cpp
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Image.h"
#include "Shader.h"
#include "VertexBuffer.h"

#include "core/memory.h"


/////////////////////////////////////////////////////////////////
// form::Image

form::Image::Image()
: shader(nullptr)
{
	ZeroObject(root_node);
	ZeroArray(corners, NUM_ROOT_VERTICES); 
}

form::Image::~Image()
{
	delete shader;
	
	for (int i = 0; i < NUM_ROOT_VERTICES; ++ i) {
		Assert (corners[i] == nullptr);
	}
}

void form::Image::InitVertices(VertexBuffer & vertices)
{
	for (int i = 0; i < NUM_ROOT_VERTICES; ++ i)
	{
		Assert(corners[i] == nullptr);
		if ((corners[i] = vertices.Alloc()) == nullptr)
		{
			Assert(false);
		}
	}
}

// If init got called, deinit must get called before d'tor.
void form::Image::DeinitVertices(VertexBuffer & vertices)
{
	for (int i = 0; i < NUM_ROOT_VERTICES; ++ i)
	{
		Vertex * vert = corners[i];
		if (vert != nullptr)
		{
			vertices.Free(vert);
			corners[i] = nullptr;
		}
	}
}

void form::Image::SetShader(form::Shader * init_shader)
{
	// Currently, no need ever to change shader.
	Assert(shader == nullptr);
	
	shader = init_shader;
}

form::Shader & form::Image::GetShader()
{
	Assert(shader != nullptr);
	return * shader;
}

