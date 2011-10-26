/*
 *  gfx/Font.cpp
 *  Crag
 *
 *  Created by John on 04/17/2010.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Font.h"
#include "Image.h"

#include "sys/App.h"


namespace 
{
	// TODO: All a big hack.
	::std::vector<gl::Vertex2dTex> vertex_buffer;
	
	int margin_hack[2] = { 8, 8 };
}


gfx::Font::Font(char const * filename, float scale)
{
	Image image;
	image.Load(filename);
	if (! image)
	{
		std::cerr << "Failed to find font file, " << filename << std::endl;
		return;
	}
	
	character_size.x = image.GetWidth() >> 4;
	character_size.y = image.GetHeight() >> 4;	
	Assert(character_size * 16 == image.GetSize());
	
	inv_scale.x = 1.f / image.GetWidth();
	inv_scale.y = 1.f / image.GetHeight();
	
	scale_factor = scale;

	image.CreateTexture(texture);
	
	gl::GenBuffer(vbo);
}

gfx::Font::~Font()
{
	gl::DeleteBuffer(vbo);
	gl::DeleteTexture(texture);
}

gfx::Font::operator bool() const
{
	return texture.IsInitialized();
}

// Warning: This function isn't thread-safe.
// Fortunately, neither is the OpenGL it uses so it's neither here nor there.
void gfx::Font::Print(char const * text, Vector2f const & position) const
{
	if (! vbo.IsInitialized())
	{
		return;
	}

	BindTexture(texture);

	BindBuffer(vbo);
	vertex_buffer.resize(0);
	GenerateVerts(text, position);
	RenderVerts();
	UnbindBuffer(vbo);

	UnbindTexture(texture);
}

void gfx::Font::GenerateVerts(char const * text, Vector2f const & position) const
{
	Vector2f p = position;
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
	gl::BufferData(vbo, num_verts, & vertex_buffer.front(), gl::STATIC_DRAW);
}

void gfx::Font::RenderVerts() const
{
	GLPP_VERIFY;

	// State
	Assert(! gl::IsEnabled(GL_LIGHTING));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	Assert(! gl::IsEnabled(GL_LIGHT0));
	gl::Enable(GL_TEXTURE_2D);
	gl::Disable(GL_CULL_FACE);
	gl::Enable(GL_BLEND);
	GLPP_CALL(glBlendEquation(GL_FUNC_ADD));
	Assert(! gl::IsEnabled(GL_DEPTH_TEST));
	gl::SetDepthMask(false);
	
	// Matrices
	gl::MatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	Vector2i resolution = sys::GetWindowSize();
	gluOrtho2D (0, resolution.x, resolution.y, 0);
	
	gl::MatrixMode (GL_MODELVIEW); 
	glLoadIdentity (); 
	glTranslatef (0.375f, 0.375f, 0.f);
	GLPP_VERIFY;
	
	// Draw VBO
	vbo.Activate();
	int num_verts = vertex_buffer.size();
	vbo.DrawQuads(0, num_verts);
	vbo.Deactivate();
	
	gl::SetDepthMask(true);
	gl::Disable(GL_TEXTURE_2D);
	gl::Enable(GL_CULL_FACE);
	gl::Disable(GL_BLEND);
}

void gfx::Font::PrintChar(char c, Vector2f & position) const
{
	unsigned int char_index = static_cast<unsigned char>(c);
	Vector2i map_pos = Vector2i(char_index & 15, char_index >> 4);
	
	Vector2f pos[2];
	Vector2f tex[2];
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
	
	Vector2i const sin_cos_table[4] = 
	{
		Vector2i(0, 0),
		Vector2i(1, 0),
		Vector2i(1, 1),
		Vector2i(0, 1)
	};
	
	gl::Vertex2dTex vert;
	for (Vector2i const * it = sin_cos_table; it != sin_cos_table + 4; ++ it)
	{
		vert.pos.x = pos[it->x].x;
		vert.pos.y = pos[it->y].y;
		vert.tex.x = tex[it->x].x;
		vert.tex.y = tex[it->y].y;
		vertex_buffer.push_back(vert);
	}
	
	position.x = pos[1].x;
}