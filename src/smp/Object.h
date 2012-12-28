//
//  Object.h
//  crag
//
//  Created by John McFarlane on 2011-05-23.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Uid.h"

namespace smp
{
	template <typename CLASS>
	class Daemon;
	
	template <typename ENGINE>
	struct ObjectInit
	{
		ObjectInit(ENGINE & init_engine, Uid init_uid)
		: engine(init_engine)
		, uid(init_uid)
		{
		}
		
		ENGINE & engine;
		Uid uid;
	};
	
	// Base class for objects used by the engines.
	// OBJECT derives directly from Object.
	template <typename OBJECT, typename ENGINE>
	class Object
	{
		OBJECT_NO_COPY(Object);

	public:
		// types
		typedef ENGINE Engine;
		typedef ::smp::Daemon<Engine> Daemon;
		typedef ObjectInit<Engine> Init;
		
		// functions
		Object(Init const & init)
		: _engine(init.engine)
		, _uid(init.uid)
		{
		}
		
		virtual ~Object()
		{ 
			VerifyObject(* this);
		}
		
#if defined(VERIFY)
		virtual void Verify() const
		{
			VerifyRef(_engine);
			ASSERT(_uid);
		}
#endif
		
		operator OBJECT & ()
		{
			OBJECT & t = * static_cast<OBJECT *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			Object & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
		operator OBJECT const & () const
		{
			OBJECT const & t = * static_cast<OBJECT const *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			Object const & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}

		Engine & GetEngine() const
		{
			return _engine;
		}
		
		Uid GetUid() const
		{
			return _uid;
		}

	private:
		// variables
		Engine & _engine;
		Uid _uid;
	};
}
