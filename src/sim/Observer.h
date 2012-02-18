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

#include "geom/Vector3.h"
#include "geom/Matrix44.h"


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
		Observer();
		~Observer();
		
		// Called from the simulation thread.
		void Init(Simulation & simulation, InitData<Observer> const & init_data);

		void AddRotation(Vector3 const & angles);
		void UpdateInput(Controller::Impulse const & impulse);

		void SetSpeed(int _speed);
		virtual void Tick(Simulation & simulation);
		virtual void UpdateModels() const;
		
	private:
		void ApplyImpulse();
		
		Vector3 impulses[2];	// [pos/rot]
		Scalar speed_factor;
#if defined(OBSERVER_LIGHT)
		gfx::Uid _light_uid;
#endif
	};
	
	// InitData struct specialization for Observer
	template <>
	struct InitData<Observer>
	{
		Vector3 center;
	};
}
