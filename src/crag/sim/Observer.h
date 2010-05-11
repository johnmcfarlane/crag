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
#include "PhysicalBody.h"
#include "UserInput.h"
#include "Controller.h"

#include "gfx/Light.h"

#include "core/Vector3.h"
#include "core/Matrix4.h"


namespace sim
{
	class Observer : public Entity
	{
	public:
		Observer();
		~Observer();
		
		void UpdateInput(Controller::Impulse const & inpulse);	// [rot/pos]

		//void SetSpeed(int _speed);
		void SetSpeedFactor(int _speed_factor);
		void Tick();
		
		Vector3f const * GetImpulse() const;
		float GetBoundingRadius() const;
		
		Vector3 const & GetPosition() const;
		void SetPosition(Vector3 const & pos);
		
		Matrix4 GetRotation() const;
		void SetRotation(Matrix4 const & rot);

		PhysicalBody * GetPhysicalBody();
		gfx::Light const & GetLight() const;

	private:
		void ApplyImpulse();
		
		Vector3f impulses[2];	// [pos/rot]
		PhysicalSphere sphere;
		float speed;
		float speed_factor;
		gfx::Light light;
	};
}
