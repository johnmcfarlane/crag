//
//  EngineBase.h
//  crag
//
//  Created by John on 2012/12/23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ObjectBase.h"
#include "Handle_Impl.h"

namespace ipc
{
	// an optional base class for the Engine classes which are managed by Daemon
	// with support for object lifetime management
	template <typename ENGINE, typename OBJECT>
	class EngineBase
	{
		OBJECT_NO_COPY(EngineBase);
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// parameter types
		using EngineType = ENGINE;
		using ObjectType = OBJECT;
		
		// the base class for objects stored in this engine
		using ObjectBaseType = ipc::ObjectBase<ObjectType, EngineType>;

		// smart pointer
		template <typename Type>
		using SharedPtr = std::shared_ptr<Type>;

		template <typename Type>
		using SharedConstPtr = std::shared_ptr<Type const>;

		template <typename Type>
		using WeakPtr = std::weak_ptr<Type>;

		template <typename Type>
		using WeakConstPtr = std::weak_ptr<Type const>;

		using ObjectSharedPtr = SharedPtr<ObjectType>;
		using ObjectSharedConstPtr = SharedConstPtr<ObjectType const>;
		using ObjectWeakPtr = WeakPtr<ObjectType>;
		using ObjectWeakConstPtr = WeakConstPtr<ObjectType const>;
		
		// object handle type
		using Handle = typename ObjectBaseType::HandleType;
		
		// object storage types
		using MapPtr = ObjectSharedPtr;
		using ObjectMap = std::unordered_map<Handle, MapPtr>;
		using Iterator = typename ObjectMap::iterator;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		EngineBase()
			: _objects()
		{
		}

		virtual ~EngineBase()
		{
			if (! IsEmpty())
			{
				DEBUG_BREAK(SIZE_T_FORMAT_SPEC " object(s) remaining", _objects.size());
			}
		}

		bool IsEmpty() const
		{
			return _objects.empty();
		}

		// object management
		ObjectSharedPtr const & GetObject(Handle handle)
		{
			auto found = _objects.find(handle);
			if (found == _objects.end())
			{
				return _null_ptr;
			}

			return found->second;
		}

		// for each map pair
		template <typename FUNCTION>
		void ForEachPair(FUNCTION f)
		{
			for (auto pair : _objects)
			{
				f(pair);
			}
		}

		// for each map pair
		template <typename FUNCTION>
		void ForEachPair(FUNCTION f) const
		{
			for (auto pair : _objects)
			{
				f(pair);
			}
		}

		// for each object
		template <typename FUNCTION>
		void ForEachObject(FUNCTION f)
		{
			ForEachPair([& f] (typename ObjectMap::value_type const & pair) {
				f(* pair.second);
			});
		}

		template <typename FUNCTION>
		void ForEachObject(FUNCTION f) const
		{
			ForEachPair([& f] (typename ObjectMap::value_type const & pair) {
				f(* pair.second);
			});
		}

		// for each object (f returns false if it is to be erased)
		template <typename FUNCTION>
		void ForEachObject_ReleaseIf(FUNCTION f)
		{
			for (auto i = _objects.begin(); i != _objects.end();)
			{
				auto pair = * i;
				if (f(* pair.second))
				{
					i = ReleaseObject(i);
				}
				else
				{
					++ i;
				}
			}
		}

		template <typename Type>
		SharedPtr<Type> CreateObject()
		{
			return CreateObject<Type>(Uid::Create());
		}
		
#if defined(WIN32)
		template <typename Type>
		SharedPtr<Type> CreateObject(Handle handle)
		{
			auto object = std::make_shared<ObjectType>(core::StaticCast<Engine>(* this));

			if (object)
			{
				AddObject(handle, object);
			}
			
			return object;
		}
#endif

		template <typename Type, typename ... PARAMETERS>
		SharedPtr<Type> CreateObject(Handle handle, PARAMETERS const & ... parameters)
		{
			auto object = std::make_shared<Type>(core::StaticCast<EngineType>(* this), parameters ...);

			if (object)
			{
				AddObject(handle, object);
			}
			
			return object;
		}

		void ReleaseObject(ObjectBaseType & object)
		{
			ReleaseObject(object.GetHandle());
		}

		void ReleaseObject(Handle handle)
		{
			ASSERT(handle);
			
			auto found = _objects.find(handle);
			if (found == _objects.end())
			{
				DEBUG_MESSAGE("Object not found");
				return;
			}

			ReleaseObject(found);
		}

		Iterator ReleaseObject(Iterator i)
		{
			auto const & object = i->second;
			OnRemoveObject(object);
			auto next = _objects.erase(i);

			return next;
		}

	private:
		virtual void OnAddObject(ObjectSharedPtr const &) { }
		virtual void OnRemoveObject(ObjectSharedPtr const &) { }

		template <typename Type>
		void AddObject(Handle handle, SharedPtr<Type> & object)
		{
			// Handle must be initialized and absent from the Engint's map
			CRAG_VERIFY_TRUE(handle);
			ASSERT(! GetObject(handle));

			// object must be valid and unassigned
			CRAG_VERIFY(object);
			CRAG_VERIFY_TRUE(! object->GetHandle());

			// assign the Handle to the object
			object->SetHandle(handle);

			// cast to the map storage type (in case Type is a derived type)
			auto _object = std::static_pointer_cast<ObjectType>(object);
			
			// store it
			_objects[handle] = _object;
			
			// invoke callback in case Engine needs to react to the addition
			OnAddObject(_object);
		}
		
	public:

		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(EngineBase, self)
			self.ForEachPair([& self] (typename ObjectMap::value_type const & pair) {
				auto const & object = * pair.second;
				CRAG_VERIFY_EQUAL(pair.first, object.GetHandle());
				CRAG_VERIFY_EQUAL(& self, & object.GetEngine());
				CRAG_VERIFY(object);
			});
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		////////////////////////////////////////////////////////////////////////////////
		// variables

	private:
		ObjectMap _objects;
		
		static ObjectSharedPtr const _null_ptr;
		static ObjectSharedConstPtr const _null_const_ptr;
	};

	template <typename ENGINE, typename OBJECT>
	typename EngineBase<ENGINE, OBJECT>::ObjectSharedPtr const 
	EngineBase<ENGINE, OBJECT>::_null_ptr;

	template <typename ENGINE, typename OBJECT>
	typename EngineBase<ENGINE, OBJECT>::ObjectSharedConstPtr const 
	EngineBase<ENGINE, OBJECT>::_null_const_ptr;
}
