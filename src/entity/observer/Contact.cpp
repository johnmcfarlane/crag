//
//  Contact.cpp
//  crag
//
//  Created by John on 2013-09-30.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Contact.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Contact friend definitions

Contact sim::ConvertOrigin(Contact const & contact, geom::abs::Vector3 const & source_origin, geom::abs::Vector3 const & destination_origin)
{
	CRAG_VERIFY(contact);
	
	switch (contact._vector_type)
	{
		default:
			DEBUG_BREAK("bad enum value, %d", int(contact._vector_type));

		case Contact::VectorType::background:
			return contact;

		case Contact::VectorType::foreground:
			return Contact(geom::Convert(contact._world_vector, source_origin, destination_origin), contact._vector_type, contact._screen_position, contact._id);
	}
}

// true iff contacts are too close together to be useful
bool sim::Collided(Contact const & contact1, Contact const & contact2)
{
	if (contact1._vector_type == contact2._vector_type)
	{
		if (contact1._world_vector == contact2._world_vector)
		{
			return true;
		}
	}
	
	if (contact1._screen_position == contact2._screen_position)
	{
		return true;
	}
	
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// sim::Contact member definitions

Contact::Contact(Vector3 const & world_vector, VectorType vector_type, Vector2 const & screen_position, SDL_FingerID id)
: _world_vector(world_vector)
, _vector_type(vector_type)
, _screen_position(screen_position)
, _id(id)
{
	CRAG_VERIFY(* this);
}

Vector3 const & Contact::GetWorldPosition() const
{
	CRAG_VERIFY(* this);
	CRAG_VERIFY_EQUAL(int(_vector_type), int(VectorType::foreground));

	return _world_vector;
}

Vector3 const & Contact::GetWorldDirection() const
{
	CRAG_VERIFY(* this);
	CRAG_VERIFY_EQUAL(int(_vector_type), int(VectorType::background));

	return _world_vector;
}

Vector3 Contact::GetWorldDirection(Vector3 const & camera_position) const
{
	CRAG_VERIFY(* this);
	
	switch (_vector_type)
	{
		default:
			DEBUG_BREAK("bad enum value, %d", int(_vector_type));

		case VectorType::background:
			return _world_vector;

		case VectorType::foreground:
			return geom::Normalized(_world_vector - camera_position);
	}
}

Contact::VectorType Contact::GetVectorType() const
{
	return _vector_type;
}

Vector2 const & Contact::GetScreenPosition() const
{
	CRAG_VERIFY(* this);
	return _screen_position;
}

void Contact::SetScreenPosition(Vector2 const & screen_position)
{
	CRAG_VERIFY(* this);
	_screen_position = screen_position;
	CRAG_VERIFY(* this);
}

SDL_FingerID Contact::GetId() const
{
	CRAG_VERIFY(* this);
	return _id;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Contact, self)
	CRAG_VERIFY(self._world_vector);

	switch (self._vector_type)
	{
		default:
			DEBUG_BREAK("bad enum value, %d", int(self._vector_type));

		case VectorType::background:
			CRAG_VERIFY_UNIT(self._world_vector, .001f);
			break;

		case VectorType::foreground:
			break;
	}
	
	CRAG_VERIFY(self._screen_position);
	CRAG_VERIFY_OP(self._id, >=, 0);
CRAG_VERIFY_INVARIANTS_DEFINE_END
