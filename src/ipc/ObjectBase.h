//
//  ipc/ObjectBase.h
//  crag
//
//  Created by John McFarlane on 2011-05-23.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Handle_Impl.h"

namespace ipc
{
	template <typename CLASS>
	class Daemon;
	
	template <typename OBJECT, typename ENGINE>
	class EngineBase;
	
	// Base class for objects used by the engines.
	// OBJECT derives directly from Object.
	template <typename OBJECT, typename ENGINE>
	class ObjectBase
	{
	public:
		// types
		using Object = OBJECT;
		using Engine = ENGINE;
		
		using EngineBase = ipc::EngineBase<Engine, Object>;
		using Daemon = Daemon<Engine> ;
		
		// friends
		friend EngineBase;

		// functions
		OBJECT_NO_COPY(ObjectBase);

		ObjectBase(Engine & engine)
		: _engine(engine)
		{
			CRAG_VERIFY(* this);
		}
				
		virtual ~ObjectBase()
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

		operator Object & ()
		{
			Object & t = core::StaticCast<Object>(* this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
		operator Object const & () const
		{
			Object const & t = core::StaticCast<Object const>(* this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase const & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}

		Engine & GetEngine() const
		{
			return _engine;
		}
		
		//  TODO: This should be GetHandle
		Uid GetUid() const
		{
			return _uid;
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(ObjectBase, object)
			CRAG_VERIFY(& object._engine);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif	// defined(CRAG_VERIFY_ENABLED)

	private:
		void SetUid(Uid uid)
		{
			static_assert(std::is_base_of<EngineBase, Engine>::value, "ENGINE isn't correctly derived from ipc::EngineBase");
			CRAG_VERIFY_TRUE(! _uid);
			
			_uid = uid;
			
			CRAG_VERIFY(* this);
		}
		
		// variables
		Engine & _engine;
		Uid _uid;
	};
}
