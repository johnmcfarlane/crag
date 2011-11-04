//
//  Message.h
//  crag
//
//  Created by John McFarlane on 5/5/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/intrusive_list.h"


namespace smp
{
	
	////////////////////////////////////////////////////////////////////////////////
	// Message base class
	//
	// Functors derived from this class are invoked upon an instance of CLASS
	// in a thread-safe manner.
	
	
	// Polymorphic base class for message wrapper.
	// Required so that messages can be listed in Actor.
	template <typename CLASS>
	class Message
	{
	public:
		virtual ~Message() 
		{ 
		}
		
		virtual void operator() (CLASS & daemon) const = 0;
	};
	
}
