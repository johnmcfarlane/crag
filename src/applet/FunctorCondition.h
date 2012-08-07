//
//  FunctorCondition.h
//  crag
//
//  Created by John McFarlane on 2012-04-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Condition.h"


namespace applet
{
	template <typename FUNCTOR>
	class FunctorCondition : public Condition
	{
	public:
		FunctorCondition(FUNCTOR functor) 
		: _functor(functor) 
		{ 
		}
		virtual bool operator() (bool hurry) override
		{
			return _functor();
		}
	private:
		FUNCTOR _functor;
	};
}
