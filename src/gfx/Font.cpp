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


namespace 
{
	// TODO: All a big hack.
	::std::vector<gfx::Font::Vertex> vertex_buffer;
	
	int margin_hack[2] = { 8, 8 };
}


////////////////////////////////////////////////////////////////////////////////
// vertex helper functions

template <>
void EnableClientState<gfx::Font::Vertex>()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

template <>
void DisableClientState<gfx::Font::Vertex>()
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

template <>
void Pointer<gfx::Font::Vertex>()
{
	gfx::VertexPointer<gfx::Font::Vertex, 2, & gfx::Font::Vertex::pos>();
	gfx::TexCoordPointer<gfx::Font::Vertex, 2, & gfx::Font::Vertex::tex>();
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Font member definitions

gfx::Font::Font(char const * filename, float scale)
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

gfx::Font::~Font()
{
	vbo.Deinit();
	texture.Deinit();
}

gfx::Font::operator bool() const
{
	return texture.IsInitialized();
}

// Warning: This function isn't thread-safe.
// Fortunately, neither is the OpenGL it uses so it's neither here nor there.
void gfx::Font::Print(char const * text, geom::Vector2f const & position) const
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

void gfx::Font::GenerateVerts(char const * text, geom::Vector2f const & position) const
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
		}
		else
		{
			PrintChar(c, p);
		}
	}
	
	size_t num_verts = vertex_buffer.size();
	vbo.BufferData(num_verts, & vertex_buffer.front(), GL_STATIC_DRAW);
}

void gfx::Font::RenderVerts() const
{
	GL_VERIFY;

	// State
	ASSERT(! IsEnabled(GL_LIGHTING));
	ASSERT(IsEnabled(GL_COLOR_MATERIAL));
	Enable(GL_TEXTURE_2D);
	Disable(GL_CULL_FACE);
	Enable(GL_BLEND);
	GL_CALL(glBlendEquation(GL_FUNC_ADD));
	ASSERT(! IsEnabled(GL_DEPTH_TEST));
	glDepthMask(false);
	
	// Matrices
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	geom::Vector2i resolution = app::GetResolution();
	gluOrtho2D (0, resolution.x, resolution.y, 0);
	
	glMatrixMode (GL_MODELVIEW); 
	glLoadIdentity (); 
	glTranslatef (0.375f, 0.375f, 0.f);
	GL_VERIFY;
	
	// Draw VBO
	vbo.Activate();
	int num_verts = vertex_buffer.size();
	vbo.DrawQuads(0, num_verts);
	vbo.Deactivate();
	
	glDepthMask(true);
	Disable(GL_TEXTURE_2D);
	Enable(GL_CULL_FACE);
	Disable(GL_BLEND);
}

void gfx::Font::PrintChar(char c, geom::Vector2f & position) const
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
	
	geom::Vector2i const sin_cos_table[4] = 
	{
		geom::Vector2i(0, 0),
		geom::Vector2i(1, 0),
		geom::Vector2i(1, 1),
		geom::Vector2i(0, 1)
	};
	
	Vertex v;
	for (geom::Vector2i const * it = sin_cos_table; it != sin_cos_table + 4; ++ it)
	{
		v.pos.x = pos[it->x].x;
		v.pos.y = pos[it->y].y;
		v.tex.x = tex[it->x].x;
		v.tex.y = tex[it->y].y;
		vertex_buffer.push_back(v);
	}
	
	position.x = pos[1].x;
}
