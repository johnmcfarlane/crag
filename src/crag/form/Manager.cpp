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
#include "form/SceneThread.h"

#include "sim/Observer.h"

#include "gfx/DebugGraphics.h"
#include "gfx/Image.h"
#include "gfx/Pov.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "core/TextureMapper.h"


namespace ANONYMOUS {

CONFIG_DEFINE (formation_emission, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
CONFIG_DEFINE (formation_ambient, gfx::Color4f, gfx::Color4f(0.05f));
CONFIG_DEFINE (formation_diffuse, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
CONFIG_DEFINE (formation_specular, float, 0.0f);
CONFIG_DEFINE (formation_shininess, float, 0.0f);

CONFIG_DEFINE (enable_multithreding, bool, true);

}


form::Manager::Manager(sim::Observer & init_observer)
: observer(init_observer)
, suspended(false)
, regenerating(false)
, scene_thread(new SceneThread (formation_set, init_observer, enable_multithreding))
, front_buffer_object(& buffer_objects [0])
, back_buffer_object(& buffer_objects [1])
, front_buffer_origin(sim::Vector3::Zero())
{
	for (int mesh_index = 0; mesh_index < 2; ++ mesh_index)
	{
		form::MeshBufferObject & mesh = buffer_objects[mesh_index];
		mesh.Init();
		mesh.Bind();
		mesh.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices);
	}
	
	InitTexture();

	VerifyObject(* this);
}

form::Manager::~Manager()
{
	scene_thread->Quit();
	
/*	for (FormationSet::iterator it = formation_set.begin(); it != formation_set.end(); ++ it) {
		Formation * formation = * it;
		delete formation;
	}*/
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

form::FormationSet const & form::Manager::GetFormations() const
{
	return formation_set;
}

void form::Manager::AdjustNumNodes(app::TimeType frame_delta, app::TimeType target_frame_delta)
{
	// Also if doing a screen capture, I shouldn't wonder.
	if (regenerating) {
		scene_thread->SetFrameRatio(1);
	}
	else {
		scene_thread->SetFrameRatio(static_cast<float>(frame_delta / target_frame_delta));
	}
}

void form::Manager::ToggleSuspended()
{
	suspended = ! suspended;
}

void form::Manager::Launch()
{
	scene_thread->Launch();
}

void form::Manager::Tick()
{
	if (scene_thread->OutOfRange()) {
		scene_thread->ResetOrigin();
		regenerating = true;
	}
	
	if (regenerating)
	{
		if (true /* done regenerating */)
		{
			regenerating = false;
		}
	}
	
	scene_thread->Tick();
}

bool form::Manager::PollMesh()
{
	if (suspended)
	{
		return false;
	}
	
	if (scene_thread->PollMesh(* back_buffer_object, front_buffer_origin))
	{
		// TODO: Do we definitely need two of these?
		std::swap(front_buffer_object, back_buffer_object);
		return true;
	}
	
	return false;
}

void form::Manager::Render(gfx::Pov const & pov, bool color) const
{	
	// State
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	
	// TODO: Minimize state changes for these too.
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular));
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	
	// Copy POV, adjust for mesh's origin and set as matrix.
	gfx::Pov formation_pov = pov;
	formation_pov.pos -= front_buffer_origin;
	GLPP_CALL(glMatrixMode(GL_MODELVIEW));
	gl::LoadMatrix(formation_pov.CalcModelViewMatrix().GetArray());
	
	
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	gl::Enable(GL_TEXTURE_2D);
	gl::Bind(& texture);
	
	
	// Draw the mesh!
	front_buffer_object->BeginDraw(color);	
	front_buffer_object->Draw();
	front_buffer_object->EndDraw();
	
	
	gl::Disable(GL_TEXTURE_2D);
	
	
	// Debug output
	if (gfx::DebugGraphics::GetVerbosity() > .8) {
		for (int i = 0; i < 2; ++ i) {
			form::MeshBufferObject const * m = buffer_objects + i;
			gfx::DebugGraphics::out << m << ' ';
			if (m == front_buffer_object) {
				gfx::DebugGraphics::out << 'f';
			}
			if (m == back_buffer_object) {
				gfx::DebugGraphics::out << 'b';
			}
			gfx::DebugGraphics::out << '\n';
		}
	}
	
	if (gfx::DebugGraphics::GetVerbosity() > .6) {
		//DebugGraphics::out << "churn:" << scene_thread->GetChurnMeasure() << '\n';
		
		//DebugGraphics::out << "regen:" << Length(scene_thread->GetObserverPos() - scene_thread->GetOrigin()) / max_observer_position_length << '\n';
		
		//DebugGraphics::out << "target:" << static_cast<float>(scene_thread->GetNumNodes()) / scene_thread->GetTargetNumNodes() << '\n';
	}
	
	if (gfx::DebugGraphics::GetVerbosity() > .05) {
		//DebugGraphics::out << "polys:" << form::Manager0::GetNumPolys() << '\n';
	}
	
	if (gfx::DebugGraphics::GetVerbosity() > .3) {
		gfx::DebugGraphics::out << "current nodes:" << (scene_thread->GetNumQuaternaAvailable() << 2) << '\n';
	}
	
	if (gfx::DebugGraphics::GetVerbosity() > .5) {
		//DebugGraphics::out << "target nodes:" << target_num_nodes << '\n';
	}
}

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
