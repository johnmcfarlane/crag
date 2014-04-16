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

namespace ipc
{
	template <typename CLASS>
	class Daemon;
	
	template <typename ENGINE>
	struct ObjectInit
	{
		OBJECT_NO_COPY(ObjectInit);

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
		typedef ::ipc::Daemon<Engine> Daemon;
		typedef ObjectInit<Engine> Init;
		
		// functions
		Object(Init const & init)
		: _engine(init.engine)
		, _uid(init.uid)
		{
		}
		
		virtual ~Object()
		{ 
			CRAG_VERIFY(* this);
		}
		
		// default allocation
		void* operator new(size_t sz) noexcept
		{
			return ::Allocate(sz, 16);
		}
		void* operator new [](size_t sz) noexcept
		{
			return ::Allocate(sz, 16);
		}
		void operator delete(void* p) noexcept
		{
			::Free(p);
		}
		void operator delete [](void* p) noexcept
		{
			::Free(p);
		}

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
		
		// TODO: This should be GetHandle
		Uid GetUid() const
		{
			return _uid;
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Object, object)
			CRAG_VERIFY(reinterpret_cast<void *>(& object._engine));
			ASSERT(object._uid);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif	// defined(CRAG_VERIFY_ENABLED)

	private:
		// variables
		Engine & _engine;
		Uid _uid;
	};
}
