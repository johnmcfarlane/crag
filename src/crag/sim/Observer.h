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

#include "geom/Vector3.h"
#include "geom/Matrix44.h"


namespace physics
{
	class SphericalBody;
}

namespace gfx
{
	class Light;
}


namespace sim
{
	class Observer : public Entity
	{
		DECLARE_SCRIPT_CLASS(Observer, Entity);

	public:
		Observer();
		~Observer();
		
		// Type-specific allocation via script.
		static void Create(Observer & observer, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(Simulation & simulation, PyObject & args);

		void AddRotation(Vector3 const & angles);
		void UpdateInput(Controller::Impulse const & impulse);

		void SetSpeedFactor(int _speed_factor);
		virtual void Tick(Simulation & simulation);
		virtual void UpdateModels() const;
		
		virtual Vector3 const & GetPosition() const;
		virtual void SetPosition(Vector3 const & pos);

	private:
		void ApplyImpulse();
		
		Vector3 impulses[2];	// [pos/rot]
		physics::SphericalBody * body;
		Scalar speed;
		Scalar speed_factor;
		gfx::Light & _light;
	};
}
