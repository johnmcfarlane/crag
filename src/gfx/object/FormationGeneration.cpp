//
//  FormationGeneration.cpp
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FormationGeneration.h"

#include "form/Mesh.h"
#include "form/Surrounding.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"
#include "gfx/Pov.h"

#include "core/ConfigEntry.h"

#include "geom/Intersection.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// FormationGeneration member definitions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(FormationGeneration, self)
	CRAG_VERIFY(self._indexed_vbo_resource);
	CRAG_VERIFY(self._properties);
CRAG_VERIFY_INVARIANTS_DEFINE_END

FormationGeneration::FormationGeneration(int max_num_quaterne)
: _indexed_vbo_resource(
	max_num_quaterne * form::Surrounding::num_verts_per_quaterna, 
	max_num_quaterne * form::Surrounding::num_indices_per_quaterna)
{
}

void FormationGeneration::SetMesh(form::Mesh const & mesh)
{
	auto & lit_mesh = mesh.GetLitMesh();
	_indexed_vbo_resource.Set(lit_mesh);
	
	_properties = mesh.GetProperties();
	
	CRAG_VERIFY(* this);
}

FormationGeneration::IndexedVboResource const & FormationGeneration::GetVboResource() const
{
	return _indexed_vbo_resource;
}

form::MeshProperties::Vector const & FormationGeneration::GetOrigin() const
{
	return _properties._origin;
}

void FormationGeneration::Draw() const
{
	_indexed_vbo_resource.Draw();
}
