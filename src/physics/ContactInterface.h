//
//  ContactInterface.h
//  crag
//
//  Created by John on 2013-10-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "core/function_ref.h"

namespace physics
{
	class ContactInterface
	{
	public:
		OBJECT_NO_COPY(ContactInterface);

		ContactInterface() = default;
		virtual ~ContactInterface() { }
		virtual void operator() (ContactGeom const * begin, ContactGeom const * end) = 0;
	};
	
	template <typename FunctionType>
	class ContactFunction : public ContactInterface
	{
		OBJECT_NO_COPY(ContactFunction);

	public:
		ContactFunction(FunctionType const & function)
		: _function(function)
		{
		}
		
		void operator() (ContactGeom const * begin, ContactGeom const * end) final
		{
			_function(begin, end);
		}
	private:
		FunctionType const & _function;
	};
}
