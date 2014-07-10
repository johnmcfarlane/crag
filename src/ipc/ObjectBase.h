//
//  ipc/ObjectBase.h
//  crag
//
//  Created by John McFarlane on 2011-05-23.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Handle.h"

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
		using ObjectType = OBJECT;
		using EngineType = ENGINE;
		using DaemonType = ::ipc::Daemon<EngineType>;
		
		using EngineBase = ipc::EngineBase<EngineType, ObjectType>;
		using HandleType = Handle<ObjectType>;
		
		// friends
		friend EngineBase;

		// functions
		OBJECT_NO_COPY(ObjectBase);

		ObjectBase(EngineType & engine)
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

		operator ObjectType & ()
		{
			ObjectType & t = core::StaticCast<ObjectType>(* this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}
		
		operator ObjectType const & () const
		{
			ObjectType const & t = core::StaticCast<ObjectType const>(* this);
			
#if ! defined(NDEBUG)
			// Check that this cast is valid.
			ObjectBase const & reverse = t;
			ASSERT(& reverse == this);
#endif
			
			return t;
		}

		EngineType & GetEngine() const
		{
			return _engine;
		}
		
		HandleType GetHandle() const
		{
			return _handle;
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(ObjectBase, object)
			CRAG_VERIFY(& object._engine);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif	// defined(CRAG_VERIFY_ENABLED)

	private:
		void SetHandle(HandleType handle)
		{
			CRAG_VERIFY_TRUE(! _handle);
			
			_handle = handle;
			
			CRAG_VERIFY(* this);
		}
		
		// variables
		EngineType & _engine;
		HandleType _handle;
	};
}
