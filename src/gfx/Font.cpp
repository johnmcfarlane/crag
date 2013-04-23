//
//  Font.cpp
//  crag
//
//  Created by John on 04/17/2010.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Font.h"
#include "Image.h"

#include "core/app.h"

using namespace gfx;

namespace 
{
	// TODO: All a big hack.
	::std::vector<Font::Vertex> vertex_buffer;
	
	int margin_hack[2] = { 8, 8 };
}


////////////////////////////////////////////////////////////////////////////////
// vertex helper functions

template <>
void EnableClientState<Font::Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
}

template <>
void DisableClientState<Font::Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Font::Vertex>()
{
	VertexAttribPointer<1, Font::Vertex, decltype(Font::Vertex::pos), & Font::Vertex::pos>();
	VertexAttribPointer<2, Font::Vertex, decltype(Font::Vertex::tex), & Font::Vertex::tex>();
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Font member definitions

Font::Font(char const * filename, float scale)
{
	Image image;
	image.Load(filename);
	if (! image)
	{
		ERROR_MESSAGE("Failed to find font file, '%s'.", filename);
		return;
	}
	
	character_size.x = image.GetWidth() >> 4;
	character_size.y = image.GetHeight() >> 4;	
	ASSERT(character_size * 16 == image.GetSize());
	
	inv_scale.x = 1.f / image.GetWidth();
	inv_scale.y = 1.f / image.GetHeight();
	
	scale_factor = scale;

	image.CreateTexture(texture);
	
	vbo.Init();
}

Font::~Font()
{
	vbo.Deinit();
	texture.Deinit();
}

Font::operator bool() const
{
	return texture.IsInitialized();
}

// Warning: This function isn't thread-safe.
// Fortunately, neither is the OpenGL it uses so it's neither here nor there.
void Font::Print(char const * text, geom::Vector2f const & position) const
{
	if (! vbo.IsInitialized())
	{
		return;
	}

	texture.Bind();

	vbo.Bind();
	vertex_buffer.resize(0);
	GenerateVerts(text, position);
	RenderVerts();
	vbo.Unbind();

	texture.Unbind();
}

void Font::GenerateVerts(char const * text, geom::Vector2f const & position) const
{
	geom::Vector2f p = position;
	while (true)
	{
		char c = * (text ++);

		if (c == '\0')
		{
			break;
		}
		else if (c == '\n')
		{
			p.x = position.x;
			p.y += scale_factor * character_size.y;
			PrintNewLine(p);
		}
		else
		{
			PrintChar(c, p);
		}
	}
	
	size_t num_verts = vertex_buffer.size();
	vbo.BufferData(num_verts, & vertex_buffer.front(), GL_STATIC_DRAW);
}

void Font::RenderVerts() const
{
	GL_VERIFY;

	// State
	ASSERT(! IsEnabled(GL_LIGHTING));
	Enable(GL_TEXTURE_2D);
	Disable(GL_CULL_FACE);
	Enable(GL_BLEND);
	ASSERT(! IsEnabled(GL_DEPTH_TEST));
	glDepthMask(false);
	
	// Draw VBO
	vbo.Activate();
	int num_verts = vertex_buffer.size();
	vbo.DrawStrip(0, num_verts);
	vbo.Deactivate();
	
	glDepthMask(true);
	Disable(GL_TEXTURE_2D);
	Enable(GL_CULL_FACE);
	Disable(GL_BLEND);
}

void Font::PrintChar(char c, geom::Vector2f & position) const
{
	unsigned int char_index = static_cast<unsigned char>(c);
	geom::Vector2i map_pos = geom::Vector2i(char_index & 15, char_index >> 4);
	
	geom::Vector2f pos[2];
	geom::Vector2f tex[2];
	for (int i = 0; i < 2; ++ i)
	{
		pos[i].x = position.x + scale_factor * static_cast<float>(character_size.x * i);
		pos[i].y = position.y + scale_factor * static_cast<float>(character_size.y * i);
		tex[i].x = inv_scale.x * (map_pos.x + i) * character_size.x;
		tex[i].y = inv_scale.y * (map_pos.y + i) * character_size.y;
	}
	//pos[0].x += margin_hack[0];
	pos[1].x -= scale_factor * (margin_hack[0] + margin_hack[1]);
	tex[0].x += inv_scale.x * margin_hack[0];
	tex[1].x -= inv_scale.x * margin_hack[1];
	
	Vertex v;
	for (auto x = 0; x != 2; ++ x)
	{
		for (auto y = 0; y != 2; ++ y)
		{
			v.pos.x = pos[x].x;
			v.pos.y = pos[y].y;
			v.tex.x = tex[x].x;
			v.tex.y = tex[y].y;
			vertex_buffer.push_back(v);
		}
	}
	
	position.x = pos[1].x;
}

void Font::PrintNewLine(geom::Vector2f & position) const
{
	vertex_buffer.push_back(vertex_buffer.back());
	vertex_buffer.push_back({position, {0, 0}});
}
