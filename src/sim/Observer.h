//
// Observer.h
// Crag
//
// Created by john on 5/13/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "Entity.h"
#include "UserInput.h"
#include "Controller.h"

#include "physics/SphericalBody.h"

#include "gfx/defs.h"

#include "geom/Matrix44.h"

namespace gfx { DECLARE_CLASS_HANDLE(Light); }	// gfx::LightHandle

//#define OBSERVER_LIGHT


namespace physics
{
	class SphericalBody;
}


namespace sim
{
	class Observer : public Entity
	{
	public:
		Observer(Init const & init, geom::abs::Vector3 const & center);
		~Observer();
		
		void AddRotation(Vector3 const & angles);
		void UpdateInput(Controller::Impulse const & impulse);

		void SetSpeed(int const & speed);
		virtual void Tick();
		virtual void UpdateModels() const;
		
	private:
		void ApplyImpulse();
		
		Vector3 impulses[2];	// [pos/rot]
		Scalar speed_factor;
#if defined(OBSERVER_LIGHT)
		gfx::LightHandle _model;
#endif
	};
}
