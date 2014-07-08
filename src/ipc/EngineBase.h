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
		
		// object storage types
		using MapPtr = ObjectSharedPtr;
		using ObjectMap = std::unordered_map<Uid, MapPtr>;
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
		template <typename Type = ObjectType>
		SharedPtr<Type> GetObject(Uid uid)
		{
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				return ObjectSharedPtr();
			}

			return found->second;
		}

		template <typename Type = ObjectType>
		SharedConstPtr<Type> GetObject(Uid uid) const
		{
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				return ObjectSharedConstPtr();
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
		void ForEachObject_DestroyIf(FUNCTION f)
		{
			for (auto i = _objects.begin(); i != _objects.end();)
			{
				auto pair = * i;
				if (f(* pair.second))
				{
					i = DestroyObject(i);
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
		SharedPtr<Type> CreateObject(Uid uid)
		{
			auto object = std::make_shared<ObjectType>(core::StaticCast<Engine>(* this));

			if (object)
			{
				AddObject(uid, object);
			}
			
			return object;
		}
#endif

		template <typename Type, typename ... PARAMETERS>
		SharedPtr<Type> CreateObject(Uid uid, PARAMETERS const & ... parameters)
		{
			auto object = std::make_shared<Type>(core::StaticCast<EngineType>(* this), parameters ...);

			if (object)
			{
				AddObject(uid, object);
			}
			
			return object;
		}

		void DestroyObject(Uid uid)
		{
			ASSERT(uid);
			
			auto found = _objects.find(uid);
			if (found == _objects.end())
			{
				DEBUG_MESSAGE("Object not found");
				return;
			}

			DestroyObject(found);
		}

		Iterator DestroyObject(Iterator destroyed)
		{
			auto const & object = destroyed->second;
			OnRemoveObject(object);
			auto next = _objects.erase(destroyed);

			return next;
		}

	private:
		virtual void OnAddObject(ObjectSharedPtr const &) { }
		virtual void OnRemoveObject(ObjectSharedPtr const &) { }

		template <typename Type>
		void AddObject(Uid uid, SharedPtr<Type> & object)
		{
			// UID must be initialized and absent from the Engint's map
			CRAG_VERIFY_TRUE(uid);
			ASSERT(! GetObject(uid));

			// object must be valid and unassigned
			CRAG_VERIFY(object);
			CRAG_VERIFY_TRUE(! object->GetUid());

			// assign the UID to the object
			object->SetUid(uid);

			// cast to the map storage type (in case Type is a derived type)
			auto _object = std::static_pointer_cast<ObjectType>(object);
			
			// store it
			_objects[uid] = _object;
			
			// invoke callback in case Engine needs to react to the addition
			OnAddObject(_object);
		}
		
	public:

		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(EngineBase, self)
			self.ForEachPair([& self] (typename ObjectMap::value_type const & pair) {
				auto const & object = * pair.second;
				CRAG_VERIFY_EQUAL(pair.first, object.GetUid());
				CRAG_VERIFY_EQUAL(& self, & object.GetEngine());
				CRAG_VERIFY(object);
			});
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		////////////////////////////////////////////////////////////////////////////////
		// variables

	private:
		ObjectMap _objects;
	};
}
