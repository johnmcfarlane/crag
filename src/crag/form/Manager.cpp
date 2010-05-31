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
#include "gfx/Pov.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"


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

void form::Manager::AdjustNumNodes(float frame_delta, float target_frame_delta)
{
	// Also if doing a screen capture, I shouldn't wonder.
	if (regenerating) {
		scene_thread->SetFrameRatio(1);
	}
	else {
		scene_thread->SetFrameRatio(frame_delta / target_frame_delta);
	}
}

void form::Manager::ToggleSuspended()
{
	Assert(false);
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
	if (scene_thread->PollMesh(* back_buffer_object, front_buffer_origin))
	{
		// TODO: There should be two form::Mesh objects - not two BO's
		std::swap(front_buffer_object, back_buffer_object);
		return true;
	}
	
	return false;
}

void form::Manager::Render(gfx::Pov const & pov, bool color) const
{	
	// State
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::DepthFunc() == GL_LEQUAL);
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	Assert(gl::DepthFunc() == GL_LEQUAL);
	
	// TODO: Minimize state changes for these too.
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular));
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	
	// Copy Pov, adjust for mesh's origin and set as matrix.
	gfx::Pov formation_pov = pov;
	formation_pov.pos -= front_buffer_origin;
	GLPP_CALL(glMatrixMode(GL_MODELVIEW));
	gl::LoadMatrix(formation_pov.CalcModelViewMatrix().GetArray());

	// Draw the mesh!
	front_buffer_object->BeginDraw(color);	
	front_buffer_object->Draw();
	front_buffer_object->EndDraw();

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
