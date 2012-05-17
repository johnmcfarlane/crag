//
//  ObjectBase.h
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
	struct ObjectBaseInit
	{
		ObjectBaseInit(ENGINE & init_engine, Uid init_uid)
		: engine(init_engine)
		, uid(init_uid)
		{
		}
		
		ENGINE & engine;
		Uid uid;
	};
	
	// Base class for objects used by the engines.
	// OBJECT derives directly from ObjectBase.
	template <typename OBJECT, typename ENGINE>
	class ObjectBase
	{
		OBJECT_NO_COPY(ObjectBase);
		
	public:
		// types
		typedef OBJECT ObjectType;
		typedef ENGINE Engine;
		typedef Daemon<Engine> Daemon;
		typedef ObjectBaseInit<Engine> Init;
		
		// functions
		ObjectBase(Init const & init)
		: _engine(init.engine)
		, _uid(init.uid)
		{
		}
		
		virtual ~ObjectBase()
		{ 
			VerifyObject(* this);
		}
		
#if defined(VERIFY)
		virtual void Verify() const
		{
			ASSERT(_uid);
		}
#endif
		
		Engine & GetEngine() const
		{
			return _engine;
		}
		
		Uid GetUid() const
		{
			return _uid;
		}
		
		operator ObjectType & ()
		{
			ObjectType & t = * static_cast<ObjectType *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
		operator ObjectType const & () const
		{
			ObjectType const & t = * static_cast<ObjectType const *>(this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase const & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
	private:
		// variables
		Engine & _engine;
		DEFINE_INTRUSIVE_LIST(ObjectBase, List);
		Uid _uid;
		
		typedef typename List::const_iterator const_iterator;
		typedef typename List::iterator iterator;
	};
}
