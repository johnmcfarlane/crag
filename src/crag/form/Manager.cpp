/*
 *  form/Manager.cpp
 *  Crag
 *
 *  Created by john on 5/23/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/Manager.h"

#include "form/Formation.h"
#include "form/scene/SceneThread.h"

#include "sim/Observer.h"

#include "gfx/Debug.h"
#include "gfx/Image.h"
#include "gfx/Pov.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "core/TextureMapper.h"


namespace 
{
	CONFIG_DEFINE (formation_emission, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_ambient, gfx::Color4f, gfx::Color4f(0.05f));
	CONFIG_DEFINE (formation_diffuse, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_specular, float, 0.0f);
	CONFIG_DEFINE (formation_shininess, float, 0.0f);

	CONFIG_DEFINE (enable_multithreding, bool, true);
	CONFIG_DEFINE (enable_dynamic_origin, bool, true);
}


form::Manager::Manager(sim::Observer & init_observer)
: observer(init_observer)
, enable_mesh_generation(true)
, scene_thread(new SceneThread (formation_set, init_observer, enable_multithreding))
{
	for (int mesh_index = 0; mesh_index < 2; ++ mesh_index)
	{
		form::MeshBufferObject & mesh = mesh_buffers[mesh_index];
		mesh.Init();
		mesh.Bind();
		mesh.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices);
	}
	
#if defined(FORM_VERTEX_TEXTURE)
	InitTexture();
#endif

	VerifyObject(* this);
}

form::Manager::~Manager()
{
	scene_thread->Quit();
}

#if VERIFY
void form::Manager::Verify() const
{
	VerifyObjectPtr(scene_thread);
}
#endif

#if DUMP
DUMP_OPERATOR_DEFINITION(form, Manager)
{
	lhs << lhs.NewLine() << "scene_thread:" << rhs.scene_thread;
	
	if (rhs.scene_thread != nullptr) {
		DumpStream indented (lhs);
		indented << (* rhs.scene_thread);
	}

	return lhs;
}
#endif

void form::Manager::AddFormation(form::Formation * formation)
{
	Assert(formation_set.find(formation) == formation_set.end());
	formation_set.insert(formation);
}

void form::Manager::RemoveFormation(form::Formation * formation)
{
	Assert(formation_set.find(formation) != formation_set.end());
	formation_set.erase(formation);
}

void form::Manager::SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta)
{
	scene_thread->SampleFrameRatio(static_cast<float>(frame_delta / target_frame_delta));
}

void form::Manager::ToggleSceneThread()
{
	scene_thread->ToggleSuspended();
}

void form::Manager::ToggleMeshGeneration()
{
	enable_mesh_generation = ! enable_mesh_generation;
}

void form::Manager::ToggleDynamicOrigin()
{
	enable_dynamic_origin = ! enable_dynamic_origin;
}

void form::Manager::ToggleFlatShaded()
{
	scene_thread->ToggleFlatShaded();
}

void form::Manager::Launch()
{
	scene_thread->Launch();
}

void form::Manager::Tick()
{
	if (enable_dynamic_origin && ! scene_thread->IsOriginOk()) 
	{
		scene_thread->ResetOrigin();
	}
	
	scene_thread->Tick();
}

void form::Manager::ForEachFormation(FormationFunctor & f) const
{
	scene_thread->ForEachFormation(f);
}

bool form::Manager::PollMesh()
{
	if (! enable_mesh_generation)
	{
		return false;
	}
	
	if (scene_thread->PollMesh(mesh_buffers.back()))
	{
		mesh_buffers.flip();
		return true;
	}
	
	return false;
}

void form::Manager::ResetRegulator()
{
	scene_thread->ResetRegulator();
}

void form::Manager::Render(gfx::Pov const & pov, bool color) const
{
	RenderFormations(pov, color);
	
	// This is an arbitrary way to make sure this fn only gets called once per frame.
	if (color)
	{
		DebugStats();
	}
}

void form::Manager::RenderFormations(gfx::Pov const & pov, bool color) const
{
	if (mesh_buffers.front().GetNumPolys() <= 0)
	{
		return;
	}
	
	// State
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular)));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess));
	GLPP_CALL(glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE));

#if defined(FORM_VERTEX_TEXTURE)
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	GLPP_CALL(gl::Enable(GL_TEXTURE_2D));
	GLPP_CALL(gl::Bind(& texture));
#else
	Assert(! gl::IsEnabled(GL_TEXTURE_2D));
#endif
	
	// Draw the mesh!
	form::MeshBufferObject const & front_buffer = mesh_buffers.front();
	front_buffer.BeginDraw(pov, color);
	front_buffer.Draw();
	front_buffer.EndDraw();
	GLPP_VERIFY;
	
#if defined(FORM_VERTEX_TEXTURE)
	GLPP_CALL(gl::Disable(GL_TEXTURE_2D));
#endif
	
	//gl::Enable(GL_COLOR_MATERIAL);
}

void form::Manager::DebugStats() const
{
	if (gfx::Debug::GetVerbosity() > .75) 
	{
		for (int i = 0; i < 2; ++ i) 
		{
			form::MeshBufferObject const * m = & mesh_buffers [i];
			gfx::Debug::out << m << ' ';
			if (m == & mesh_buffers.front()) 
			{
				gfx::Debug::out << 'f';
			}
			if (m == & mesh_buffers.back()) 
			{
				gfx::Debug::out << 'b';
			}
			gfx::Debug::out << '\n';
		}
	}
	
	if (gfx::Debug::GetVerbosity() > .15) 
	{
		// TODO: Stop this flickering.
		gfx::Debug::out << "polys:" << mesh_buffers.front().GetNumPolys() << '\n';
	}
	
	if (! enable_mesh_generation && gfx::Debug::GetVerbosity() > .0) 
	{
		gfx::Debug::out << "disabled: mesh generation\n";
	}
	
	if (! enable_dynamic_origin && gfx::Debug::GetVerbosity() > .05) 
	{
		gfx::Debug::out << "disabled: dynamic origin\n";
	}
}

#if defined(FORM_VERTEX_TEXTURE)
bool form::Manager::InitTexture()
{
	gfx::Image image;
	Vector2i image_size(128, 128);
	image.Create(image_size);
	
	TextureMapper mapper(image_size, 0);
	
	std::cout << mapper.GetMapping(Vector2f(0, 0)).x << mapper.GetMapping(Vector2f(0, 0)).y;
	std::cout << mapper.GetMapping(Vector2f(1, 0)).x << mapper.GetMapping(Vector2f(1, 0)).y;
	std::cout << mapper.GetMapping(Vector2f(0, 1)).x << mapper.GetMapping(Vector2f(0, 1)).y;
	
	//float root_two = Sqrt(2.);
	Vector2i pos;
	Vector2f f_pos;
	for (pos.y = 0; pos.y < image_size.y; ++ pos.y)
	{
		f_pos.y = pos.y;
		for (pos.x = 0; pos.x < image_size.x; ++ pos.x)
		{
			f_pos.x = pos.x;
			
			Vector2f equalateral_pos = mapper.GetMapping(f_pos);
			
			float i = Min(Length(mapper.GetCorner(1) - equalateral_pos), 1.f);
			
			image.SetPixel(pos, gfx::Color4f(i, i, i));
			
			/*float r = Min(.5f - Length(mapper.GetCorner(0) - equalateral_pos) * .5f, 1.f);
			float g = Min(.5f - Length(mapper.GetCorner(1) - equalateral_pos) * .5f, 1.f);
			float b = Min(.5f - Length(mapper.GetCorner(2) - equalateral_pos) * .5f, 1.f);
		
			image.SetPixel(pos, gfx::Color4f(r, g, b));*/
		}
	}
	
	if (! image.CreateTexture(texture))
	{
		return false;
	}
	
	return true;
}
#endif	// defined(FORM_VERTEX_TEXTURE)
