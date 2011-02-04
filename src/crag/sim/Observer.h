/*
 *  Observer.h
 *  Crag
 *
 *  Created by john on 5/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Entity.h"
#include "UserInput.h"
#include "Controller.h"

#include "physics/SphericalBody.h"

#include "gfx/Light.h"

#include "geom/Vector3.h"
#include "geom/Matrix4.h"


namespace physics
{
	class SphericalBody;
}


namespace sim
{
	class Observer : public Entity
	{
	public:
		Observer(SimulationPtr const & s, sim::Vector3 const & init_pos);
		~Observer();
		
		// Type-specific allocation via script.
		static Observer * Create(PyObject * args);

		void UpdateInput(Controller::Impulse const & inpulse);	// [rot/pos]

		void SetSpeedFactor(int _speed_factor);
		void Tick(Universe const & universe);
		
		virtual Vector3 const & GetPosition() const;
		void SetPosition(Vector3 const & pos);

	private:
		void ApplyImpulse();
		
		Vector3 impulses[2];	// [pos/rot]
		physics::SphericalBody sphere;
		Scalar speed;
		Scalar speed_factor;
		gfx::Light light;
	};
}
