//
//  Contact.h
//  crag
//
//  Created by John on 2013-09-30.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "geom/origin.h"

#include "sim/defs.h"

namespace sim
{
	////////////////////////////////////////////////////////////////////////////////
	// types

	// forward declarations
	class Contact;

	// called on a contact when the origin changes
	Contact ConvertOrigin(Contact const & contact, geom::abs::Vector3 const & source_origin, geom::abs::Vector3 const & destination_origin);

	// true iff contacts are too close together to be useful
	bool Collided(Contact const & contact1, Contact const & contact2);

	////////////////////////////////////////////////////////////////////////////////
	// sim::Contact definition

	class Contact
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		enum class VectorType
		{
			foreground,
			background
		};

		////////////////////////////////////////////////////////////////////////////////
		// functions

		friend Contact ConvertOrigin(Contact const & contact, geom::abs::Vector3 const & source_origin, geom::abs::Vector3 const & destination_origin);
		friend bool Collided(Contact const & contact1, Contact const & contact2);

		Contact() = default;
		Contact(Vector3 const & world_vector, VectorType vector_type, Vector2 const & screen_position, SDL_FingerID id);

		Vector3 const & GetWorldPosition() const;
		Vector3 const & GetWorldDirection() const;
		Vector3 GetWorldDirection(Vector3 const & camera_position) const;

		VectorType GetVectorType() const;

		Vector2 const & GetScreenPosition() const;
		void SetScreenPosition(Vector2 const & screen_position);

		SDL_FingerID GetId() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Contact);

	private:
		// initial direction from camera to point of contact between contact and screen
		// OR the direction from camera to background, i.e. infinity
		Vector3 _world_vector;

		// dictates whether _world_vector is position or direction
		VectorType _vector_type;

		// current screen position
		Vector2 _screen_position;

		// SDL ID of contact
		SDL_FingerID _id = invalid_id;

		static SDL_FingerID constexpr invalid_id = -1;
	};
}
